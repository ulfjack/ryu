// Copyright 2018 Ulf Adams
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

#include "ryu/d2s.h"

static inline int32_t max_int32(int32_t a, int32_t b) {
  return a > b ? a : b;
}

static inline int32_t pow5Factor(uint64_t value) {
  for (int32_t count = 0; value > 0; ++count) {
    if (value - 5 * (value / 5) != 0) {
      return count;
    }
    value /= 5;
  }
  return 0;
}

// Returns true if value divides 5^p.
static inline bool multipleOfPowerOf5(uint64_t value, int32_t p) {
  // I tried a case distinction on p, but there was no performance difference.
  return pow5Factor(value) >= p;
}

// We need a 64x128 bit multiplication and a subsequent 128-bit shift.
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
// 1. Best case: the compiler exposes a 128-bit type
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
//    a. Ignore this fact and just implement the intrinsics manually
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#if defined(HAS_UINT128)

// Best case: use 128-bit type.
static inline uint64_t mulShift(uint64_t m, const uint64_t* mul, int32_t j) {
  uint128_t b0 = ((uint128_t) m) * mul[0];
  uint128_t b2 = ((uint128_t) m) * mul[1];
  return (uint64_t) (((b0 >> 64) + b2) >> (j - 64));
}

static inline uint64_t mulShiftAll(
    uint64_t m, const uint64_t* mul, int32_t j, uint64_t* vp, uint64_t* vm, uint32_t mmShift) {
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

static inline uint64_t mulShift(uint64_t m, const uint64_t* mul, int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                             // 128
  uint64_t low1 = umul128(m, mul[1], &high1); // 64
  uint64_t high0;                             // 64
  umul128(m, mul[0], &high0);                 // 0
  uint64_t sum = high0 + low1;
  if (sum < high0) {
    ++high1; // overflow into high1
  }
  return shiftright128(sum, high1, j - 64);
}

static inline uint64_t mulShiftAll(
    uint64_t m, const uint64_t* mul, int32_t j, uint64_t* vp, uint64_t* vm, uint32_t mmShift) {
  *vp = mulShift(4 * m + 2, mul, j);
  *vm = mulShift(4 * m - 1 - mmShift, mul, j);
  return mulShift(4 * m, mul, j);
}

#else // !defined(HAS_UINT128) && !defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t mulShiftAll(
    uint64_t m, const uint64_t* mul, int32_t j, uint64_t* vp, uint64_t* vm, uint32_t mmShift) {
  m <<= 1;
  // m is maximum 55 bits
  uint64_t tmp;
  uint64_t lo = umul128(m, mul[0], &tmp);
  uint64_t hi;
  uint64_t mid = tmp + umul128(m, mul[1], &hi);
  hi += mid < tmp; // overflow into hi

  uint64_t lo2 = lo + mul[0];
  uint64_t mid2 = mid + mul[1] + (lo2 < lo);
  uint64_t hi2 = hi + (mid2 < mid);
  *vp = shiftright128(mid2, hi2, j - 64 - 1);

  if (mmShift == 1) {
    uint64_t lo3 = lo - mul[0];
    uint64_t mid3 = mid - mul[1] - (lo3 > lo);
    uint64_t hi3 = hi - (mid3 > mid);
    *vm = shiftright128(mid3, hi3, j - 64 - 1);
  } else {
    uint64_t lo3 = lo + lo;
    uint64_t mid3 = mid + mid + (lo3 < lo);
    uint64_t hi3 = hi + hi + (mid3 < mid);
    uint64_t lo4 = lo3 - mul[0];
    uint64_t mid4 = mid3 - mul[1] - (lo4 > lo3);
    uint64_t hi4 = hi3 - (mid4 > mid3);
    *vm = shiftright128(mid4, hi4, j - 64);
  }

  return shiftright128(mid, hi, j - 64 - 1);
}

#endif // HAS_64_BIT_INTRINSICS

static inline uint32_t decimalLength(uint64_t v) {
  // This is slightly faster than a loop. For a random set of numbers, the
  // average length is 17.4 digits, so we check high-to-low.
  // 2^63 - 1 is 19 decimal digits, while 2^64 - 1 is 20 decimal digits.
  // Function precondition: v is not a 20-digit number.
  if (v >= 1000000000000000000L) { return 19; }
  if (v >= 100000000000000000L) { return 18; }
  if (v >= 10000000000000000L) { return 17; }
  if (v >= 1000000000000000L) { return 16; }
  if (v >= 100000000000000L) { return 15; }
  if (v >= 10000000000000L) { return 14; }
  if (v >= 1000000000000L) { return 13; }
  if (v >= 100000000000L) { return 12; }
  if (v >= 10000000000L) { return 11; }
  if (v >= 1000000000L) { return 10; }
  if (v >= 100000000L) { return 9; }
  if (v >= 10000000L) { return 8; }
  if (v >= 1000000L) { return 7; }
  if (v >= 100000L) { return 6; }
  if (v >= 10000L) { return 5; }
  if (v >= 1000L) { return 4; }
  if (v >= 100L) { return 3; }
  if (v >= 10L) { return 2; }
  return 1;
}

void d2s_buffered(double f, char* result) {
  // Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
  uint32_t mantissaBits = DOUBLE_MANTISSA_BITS;
  uint32_t exponentBits = DOUBLE_EXPONENT_BITS;
  uint32_t offset = (1u << (exponentBits - 1)) - 1;

  uint64_t bits = 0;
  // This only works on little-endian architectures.
  memcpy(&bits, &f, sizeof(double));

  // Decode bits into sign, mantissa, and exponent.
  bool sign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
  uint64_t ieeeMantissa = bits & ((1ull << mantissaBits) - 1);
  uint32_t ieeeExponent = (uint32_t) ((bits >> mantissaBits) & ((1u << exponentBits) - 1));

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
  if (ieeeExponent == ((1u << exponentBits) - 1u)) {
    strcpy(result, (ieeeMantissa != 0) ? "NaN" : sign ? "-Infinity" : "Infinity");
    return;
  } else if (ieeeExponent == 0) {
    if (ieeeMantissa == 0) {
      strcpy(result, sign ? "-0E0" : "0E0");
      return;
    }
    // We subtract 2 so that the bounds computation has 2 additional bits.
    e2 = 1 - offset - mantissaBits - 2;
    m2 = ieeeMantissa;
  } else {
    e2 = ieeeExponent - offset - mantissaBits - 2;
    m2 = (1ull << mantissaBits) | ieeeMantissa;
  }
  bool even = (m2 & 1) == 0;
  bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("S=%s E=%d M=%" PRIu64 "\n", sign ? "-" : "+", e2 + 2, m2);
#endif

  // Step 2: Determine the interval of legal decimal representations.
  uint64_t mv = 4 * m2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  uint32_t mmShift = (m2 != (1ull << mantissaBits)) || (ieeeExponent <= 1);
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
    int32_t q = max_int32(0, ((int32_t) ((e2 * DOUBLE_LOG10_2_NUMERATOR) / DOUBLE_LOG10_2_DENOMINATOR)) - 1);
    e10 = q;
    int32_t k = DOUBLE_POW5_INV_BITCOUNT + double_pow5bits(q) - 1;
    int32_t i = -e2 + q + k;
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
    int32_t q = max_int32(0, ((int32_t) ((-e2 * DOUBLE_LOG10_5_NUMERATOR) / DOUBLE_LOG10_5_DENOMINATOR)) - 1);
    e10 = q + e2;
    int32_t i = -e2 - q;
    int32_t k = double_pow5bits(i) - DOUBLE_POW5_BITCOUNT;
    int32_t j = q - k;
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
      // <=> mv & ((1 << (q-1)) - 1) == 0
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
  uint32_t vplength = decimalLength(vp);
  int32_t exp = e10 + vplength - 1;

  uint32_t removed = 0;
  uint8_t lastRemovedDigit = 0;
  uint64_t output;
  // On average, we remove ~2 digits.
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely (<1%).
    while (vp / 10 > vm / 10) {
      // The compiler does not realize that vm % 10 can be computed from vm / 10
      // as vm - (vm / 10) * 10.
      vmIsTrailingZeros &= vm - (vm / 10) * 10 == 0; // vm % 10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      uint64_t nvr = vr / 10;
      lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
      vr = nvr;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    // Same as above; use vm % 10 == vm - (vm / 10) * 10.
    if (vmIsTrailingZeros) {
      while (vm - (vm / 10) * 10 == 0) {
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        uint64_t nvr = vr / 10;
        lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
        vr = nvr;
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
      uint64_t nvr = vr / 10;
      lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
      vr = nvr;
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
  // The average output length is 16.38 digits.
  uint32_t olength = vplength - removed;
#ifdef RYU_DEBUG
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("O=%" PRIu64 "\n", output);
  printf("OLEN=%d\n", olength);
  printf("EXP=%d\n", exp);
#endif

  // Step 5: Print the decimal representation.
  int index = 0;
  if (sign) {
    result[index++] = '-';
  }

#ifndef NO_DIGIT_TABLE
  // Print decimal digits after the decimal point.
  uint32_t i = 0;
#if defined(_M_IX86) || defined(_M_ARM)
  // 64-bit division is inefficient on 32-bit platforms.
  uint32_t output2;
  while ((output >> 32) != 0) {
    // Expensive 64-bit division.
    output2 = (uint32_t) (output - 1000000000 * (output / 1000000000)); // output % 1000000000
    output /= 1000000000;

    // Cheap 32-bit divisions.
    uint32_t c = output2 - 10000 * (output2 / 10000); // output2 % 10000;
    output2 /= 10000;
    uint32_t d = output2 - 10000 * (output2 / 10000); // output2 % 10000;
    output2 /= 10000;
    uint32_t c0 = (c % 100) << 1;
    uint32_t c1 = (c / 100) << 1;
    uint32_t d0 = (d % 100) << 1;
    uint32_t d1 = (d / 100) << 1;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
    memcpy(result + index + olength - i - 5, DIGIT_TABLE + d0, 2);
    memcpy(result + index + olength - i - 7, DIGIT_TABLE + d1, 2);
    result[index + olength - i - 8] = (char) ('0' + output2);
    i += 9;
  }
  output2 = (uint32_t) output;
#else // ^^^ known 32-bit platforms ^^^ / vvv other platforms vvv
  // 64-bit division is efficient on 64-bit platforms.
  uint64_t output2 = output;
#endif // ^^^ other platforms ^^^
  while (output2 >= 10000) {
    uint32_t c = (uint32_t) (output2 - 10000 * (output2 / 10000)); // output2 % 10000;
    output2 /= 10000;
    uint32_t c0 = (c % 100) << 1;
    uint32_t c1 = (c / 100) << 1;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (output2 >= 100) {
    uint32_t c = (uint32_t) ((output2 - 100 * (output2 / 100)) << 1); // (output2 % 100) << 1;
    output2 /= 100;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (output2 >= 10) {
    uint32_t c = (uint32_t) (output2 << 1);
    result[index + olength - i] = DIGIT_TABLE[c + 1];
    result[index] = DIGIT_TABLE[c];
  } else {
    // Print the leading decimal digit.
    result[index] = (char) ('0' + output2);
  }
#else
  // Print decimal digits after the decimal point.
  for (uint32_t i = 0; i < olength - 1; ++i) {
    uint32_t c = output % 10; output /= 10;
    result[index + olength - i] = (char) ('0' + c);
  }
  // Print the leading decimal digit.
  result[index] = '0' + output % 10;
#endif // NO_DIGIT_TABLE

  // Print decimal point if needed.
  if (olength > 1) {
    result[index + 1] = '.';
    index += olength + 1;
  } else {
    ++index;
  }

  // Print the exponent.
  result[index++] = 'E';
  if (exp < 0) {
    result[index++] = '-';
    exp = -exp;
  }

#ifndef NO_DIGIT_TABLE
  if (exp >= 100) {
    result[index++] = (char) ('0' + exp / 100);
    exp = exp - 100 * (exp / 100);
    memcpy(result + index, DIGIT_TABLE + (2 * exp), 2);
    index += 2;
  } else if (exp >= 10) {
    memcpy(result + index, DIGIT_TABLE + (2 * exp), 2);
    index += 2;
  } else {
    result[index++] = (char) ('0' + exp);
  }
#else
  if (exp >= 100) {
    result[index++] = (char) ('0' + exp / 100);
  }
  if (exp >= 10) {
    result[index++] = '0' + (exp / 10) % 10;
  }
  result[index++] = '0' + exp % 10;
#endif // NO_DIGIT_TABLE

  // Terminate the string.
  result[index++] = '\0';
}

char* d2s(double f) {
  char* result = (char*) calloc(25, sizeof(char));
  d2s_buffered(f, result);
  return result;
}
