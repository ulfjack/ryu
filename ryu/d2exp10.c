// Copyright 2018 Ulf Adams and contributors
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// Runtime compiler options:
// -DRYU_DEBUG Generate verbose debugging output to stdout.
//
// -DRYU_ONLY_64_BIT_OPS Avoid using uint128_t or 64-bit intrinsics. Slower,
//     depending on your compiler.
//
// -DRYU_OPTIMIZE_SIZE Use smaller lookup tables. Instead of storing every
//     required power of 5, only store every 26th entry, and compute
//     intermediate values with a multiplication. This reduces the lookup table
//     size by about 10x (only one case, and only double) at the cost of some
//     performance. Currently requires MSVC intrinsics.

#include "ryu/ryu.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_DIGIT_TABLE
#include "ryu/digit_table.h"
#endif

#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif

// ABSL avoids uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS)
#define HAS_UINT128
#elif defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS) && defined(_M_X64) \
  && !defined(__clang__) // https://bugs.llvm.org/show_bug.cgi?id=37755
#define HAS_64_BIT_INTRINSICS
#endif

#include "ryu/common.h"
#include "ryu/d2s.h"
#include "ryu/to_decimal.h"

static inline int32_t pow5Factor(uint64_t value) {
  for (int32_t count = 0; value > 0; ++count) {
    if (value % 5 != 0) {
      return count;
    }
    value /= 5;
  }
  return 0;
}

// Returns true if value is divisible by 5^p.
static inline bool multipleOfPowerOf5(const uint64_t value, const int32_t p) {
  // I tried a case distinction on p, but there was no performance difference.
  return pow5Factor(value) >= p;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55+124=179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179-115=64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j-64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x86-64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x86-64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#if defined(HAS_UINT128)

// Best case: use 128-bit type.
static inline uint64_t mulShift(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  const uint128_t b0 = ((uint128_t) m) * mul[0];
  const uint128_t b2 = ((uint128_t) m) * mul[1];
  return (uint64_t) (((b0 >> 64) + b2) >> (j - 64));
}

static inline uint64_t mulShiftAll(
    const uint64_t m, const uint64_t* const mul, const int32_t j, uint64_t* const vp, uint64_t* const vm, const uint32_t mmShift) {
//  m <<= 2;
//  uint128_t b0 = ((uint128_t) m) * mul[0]; // 0
//  uint128_t b2 = ((uint128_t) m) * mul[1]; // 64
//
//  uint128_t hi = (b0 >> 64) + b2;
//  uint128_t lo = b0 & 0xffffffffffffffffull;
//  uint128_t factor = (((uint128_t) mul[1]) << 64) + mul[0];
//  uint128_t vpLo = lo + (factor << 1);
//  *vp = (uint64_t) ((hi + (vpLo >> 64)) >> (j - 64));
//  uint128_t vmLo = lo - (factor << mmShift);
//  *vm = (uint64_t) ((hi + (vmLo >> 64) - (((uint128_t) 1ull) << 64)) >> (j - 64));
//  return (uint64_t) (hi >> (j - 64));
  *vp = mulShift(4 * m + 2, mul, j);
  *vm = mulShift(4 * m - 1 - mmShift, mul, j);
  return mulShift(4 * m, mul, j);
}

#elif defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t mulShift(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                                   // 128
  const uint64_t low1 = umul128(m, mul[1], &high1); // 64
  uint64_t high0;                                   // 64
  umul128(m, mul[0], &high0);                       // 0
  const uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  return shiftright128(sum, high1, j - 64);
}

static inline uint64_t mulShiftAll(
    const uint64_t m, const uint64_t* const mul, const int32_t j, uint64_t* const vp, uint64_t* const vm, const uint32_t mmShift) {
  *vp = mulShift(4 * m + 2, mul, j);
  *vm = mulShift(4 * m - 1 - mmShift, mul, j);
  return mulShift(4 * m, mul, j);
}

#else // !defined(HAS_UINT128) && !defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t mulShiftAll(
    uint64_t m, const uint64_t* const mul, const int32_t j, uint64_t* const vp, uint64_t* const vm, const uint32_t mmShift) {
  m <<= 1;
  // m is maximum 55 bits
  uint64_t tmp;
  const uint64_t lo = umul128(m, mul[0], &tmp);
  uint64_t hi;
  const uint64_t mid = tmp + umul128(m, mul[1], &hi);
  hi += mid < tmp; // overflow into hi

  const uint64_t lo2 = lo + mul[0];
  const uint64_t mid2 = mid + mul[1] + (lo2 < lo);
  const uint64_t hi2 = hi + (mid2 < mid);
  *vp = shiftright128(mid2, hi2, j - 64 - 1);

  if (mmShift == 1) {
    const uint64_t lo3 = lo - mul[0];
    const uint64_t mid3 = mid - mul[1] - (lo3 > lo);
    const uint64_t hi3 = hi - (mid3 > mid);
    *vm = shiftright128(mid3, hi3, j - 64 - 1);
  } else {
    const uint64_t lo3 = lo + lo;
    const uint64_t mid3 = mid + mid + (lo3 < lo);
    const uint64_t hi3 = hi + hi + (mid3 < mid);
    const uint64_t lo4 = lo3 - mul[0];
    const uint64_t mid4 = mid3 - mul[1] - (lo4 > lo3);
    const uint64_t hi4 = hi3 - (mid4 > mid3);
    *vm = shiftright128(mid4, hi4, j - 64);
  }

  return shiftright128(mid, hi, j - 64 - 1);
}

#endif // HAS_64_BIT_INTRINSICS

struct exp10double d2exp10(double f) {
  // Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
  const uint32_t mantissaBits = DOUBLE_MANTISSA_BITS;
  const uint32_t exponentBits = DOUBLE_EXPONENT_BITS;
  const uint32_t offset = (1u << (exponentBits - 1)) - 1;

  uint64_t bits = 0;
  // This only works on little-endian architectures.
  memcpy(&bits, &f, sizeof(double));

  // Decode bits into sign, mantissa, and exponent.
  const bool sign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
  const uint64_t ieeeMantissa = bits & ((1ull << mantissaBits) - 1);
  const uint32_t ieeeExponent = (uint32_t) ((bits >> mantissaBits) & ((1u << exponentBits) - 1));

#ifdef RYU_DEBUG
  printf("IN=");
  for (int32_t bit = 63; bit >= 0; --bit) {
    printf("%d", (int) ((bits >> bit) & 1));
  }
  printf("\n");
#endif

  int32_t e2;
  uint64_t m2;
  // Case distinction; exit early for the easy cases.
  if (ieeeExponent == ((1u << exponentBits) - 1u) || (ieeeExponent == 0 && ieeeMantissa == 0)) {
    return (struct exp10double) {
      .sign = sign,
      .exp10 = ieeeExponent ? INT16_MAX : 0,
      .value = ieeeMantissa,
    };
  } else if (ieeeExponent == 0) {
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - offset - mantissaBits - 2;
    m2 = ieeeMantissa;
  } else {
    e2 = ieeeExponent - offset - mantissaBits - 2;
    m2 = (1ull << mantissaBits) | ieeeMantissa;
  }
  const bool even = (m2 & 1) == 0;
  const bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("S=%s E=%d M=%" PRIu64 "\n", sign ? "-" : "+", e2 + 2, m2);
#endif

  // Step 2: Determine the interval of legal decimal representations.
  const uint64_t mv = 4 * m2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  const uint32_t mmShift = (m2 != (1ull << mantissaBits)) || (ieeeExponent <= 1);
  // We would compute mp and mm like this:
//  uint64_t mp = 4 * m2 + 2;
//  uint64_t mm = mv - 1 - mmShift;

  // Step 3: Convert to a decimal power base using 128-bit arithmetic.
  uint64_t vr, vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  if (e2 >= 0) {
    // I tried special-casing q == 0, but there was no effect on performance.
    // This expression is slightly faster than max(0, log10Pow2(e2) - 1).
    const int32_t q = log10Pow2(e2) - (e2 > 3);
    e10 = q;
    const int32_t k = DOUBLE_POW5_INV_BITCOUNT + pow5bits(q) - 1;
    const int32_t i = -e2 + q + k;
#if defined(RYU_OPTIMIZE_SIZE)
    uint64_t pow5[2];
    double_computeInvPow5(q, pow5);
    vr = mulShiftAll(m2, pow5, i, &vp, &vm, mmShift);
#else
    vr = mulShiftAll(m2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 2^%d / 10^%d\n", mv, e2, q);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 21) {
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
        vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
      } else {
        if (acceptBounds) {
          // Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
          // <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
          // <=> true && pow5Factor(mm) >= q, since e2 >= q.
          vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
        } else {
          // Same as min(e2 + 1, pow5Factor(mp)) >= q.
          vp -= multipleOfPowerOf5(mv + 2, q);
        }
      }
    }
  } else {
    // This expression is slightly faster than max(0, log10Pow5(-e2) - 1).
    const int32_t q = log10Pow5(-e2) - (-e2 > 1);
    e10 = q + e2;
    const int32_t i = -e2 - q;
    const int32_t k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
    const int32_t j = q - k;
#if defined(RYU_OPTIMIZE_SIZE)
    uint64_t pow5[2];
    double_computePow5(i, pow5);
    vr = mulShiftAll(m2, pow5, j, &vp, &vm, mmShift);
#else
    vr = mulShiftAll(m2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mmShift);
#endif
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 5^%d / 10^%d\n", mv, -e2, q);
    printf("%d %d %d %d\n", q, i, k, j);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 1) {
      vrIsTrailingZeros = (~((uint32_t) mv) & 1) >= (uint32_t) q;
      if (acceptBounds) {
        vmIsTrailingZeros = (~((uint32_t) (mv - 1 - mmShift)) & 1) >= (uint32_t) q;
      } else {
        --vp;
      }
    } else if (q < 63) { // TODO(ulfjack): Use a tighter bound here.
      // We need to compute min(ntz(mv), pow5Factor(mv) - e2) >= q-1
      // <=> ntz(mv) >= q-1  &&  pow5Factor(mv) - e2 >= q-1
      // <=> ntz(mv) >= q-1
      // <=> (mv & ((1 << (q-1)) - 1)) == 0
      // We also need to make sure that the left shift does not overflow.
      vrIsTrailingZeros = (mv & ((1ull << (q - 1)) - 1)) == 0;
#ifdef RYU_DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

  // Step 4: Find the shortest decimal representation in the interval of legal representations.
  uint32_t removed = 0;
  uint8_t lastRemovedDigit = 0;
  uint64_t output;
  // On average, we remove ~2 digits.
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (<1%).
    while (vp / 10 > vm / 10) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=23106
      // The compiler does not realize that vm % 10 can be computed from vm / 10
      // as vm - (vm / 10) * 10.
      vmIsTrailingZeros &= vm - (vm / 10) * 10 == 0;
#else
      vmIsTrailingZeros &= vm % 10 == 0;
#endif
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (uint8_t) (vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    if (vmIsTrailingZeros) {
      while (vm % 10 == 0) {
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (uint8_t) (vr % 10);
        vr /= 10;
        vp /= 10;
        vm /= 10;
        ++removed;
      }
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    if (vrIsTrailingZeros && (lastRemovedDigit == 5) && (vr % 2 == 0)) {
      // Round down not up if the number ends in X50000.
      lastRemovedDigit = 4;
    }
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr +
        ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || (lastRemovedDigit >= 5));
  } else {
    // Specialized for the common case (>99%).
    while (vp / 10 > vm / 10) {
      lastRemovedDigit = (uint8_t) (vr % 10);
      vr /= 10;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm) || (lastRemovedDigit >= 5));
  }

  struct exp10double result = {
    .sign = sign,
    .value = output,
    .exp10 = e10 + removed
  };

#ifdef RYU_DEBUG
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("O=%" PRIu64 "\n", output);
  printf("EXP=%d\n", result.exp);
#endif

  return result;
}
