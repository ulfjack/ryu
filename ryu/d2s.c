// Copyright 2018 Ulf Adams
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Compile with -DMATCH_GRISU3_OUTPUT to match Grisu3 output perfectly.
// Compile with -DDEBUG to get very verbose debugging output to stdout.

#include "ryu/ryu.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ryu/dtable.h"

//#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#endif

// ABSL avoids uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_WIN32)

#define HAS_UINT128
typedef __uint128_t uint128_t;
// FAST_POW5 requires uint128, so it's only available on non-Win32 platforms.
#define FAST_POW5

#elif defined(_MSC_VER)

#define HAS_64_BIT_INTRINSICS
// MSVC calls it __inline, not inline in C mode.
#define inline __inline

#else

#define USE_MANUAL_64_BIT_MUL

#endif

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11

#define LOG10_2_DENOMINATOR 10000000L
#define LOG10_2_NUMERATOR 3010299L // LOG10_2_DENOMINATOR * log_10(2)
#define LOG10_5_DENOMINATOR 10000000L
#define LOG10_5_NUMERATOR 6989700L // LOG10_5_DENOMINATOR * log_10(5)
#define LOG2_5_DENOMINATOR 10000000L
#define LOG2_5_NUMERATOR 23219280L // LOG2_5_DENOMINATOR * log_2(5)

#ifndef NO_DIGIT_TABLE
static const char DIGIT_TABLE[200] = {
  '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
  '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
  '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
  '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
  '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
  '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
  '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
  '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
  '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
  '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};
#endif // NO_DIGIT_TABLE

static inline int32_t max_uint32(int32_t a, int32_t b) {
  return a > b ? a : b;
}

#ifdef FAST_POW5
static uint64_t POW5[22] = {
  1u, 5u, 25u, 125u, 625u, 3125u, 15625u, 78125u, 390625u, 1953125u, 9765625u,
  48828125u, 244140625u, 1220703125u, 6103515625u, 30517578125u, 152587890625u,
  762939453125u, 3814697265625u, 19073486328125u, 95367431640625u,
  476837158203125u
};

// x / POW5[i] == (x * POW5_MUL[i]) >> (64 + POW5_SHR[i])
static uint64_t POW5_MUL[22] = {
0u, 3689348814741910374u, 737869762948382074u, 147573952589676414u,
118059162071741131u, 94447329657392905u, 18889465931478581u, 60446290980731459u,
48357032784585167u, 19342813113834067u, 30948500982134507u, 49517601571415211u,
39614081257132169u, 126765060022822941u, 101412048018258353u, 40564819207303341u,
32451855365842673u, 51922968585348277u, 83076749736557243u, 33230699894622897u,
53169119831396635u, 10633823966279327u,
};

static uint64_t POW5_SHR[22] = {
  0, 0, 0, 0, 2, 4, 4, 8, 10, 11, 14, 17, 19, 23, 25, 26, 28, 31, 34, 35, 38, 38,
};

static inline bool multipleOfPowerOf5(uint64_t value, int32_t p) {
  if (p == 0) return true;
  uint64_t div = ((value * (uint128_t) POW5_MUL[p]) >> 64) >> POW5_SHR[p];
  return (value - POW5[p] * div) == 0;
}
#else // FAST_POW5
static inline uint32_t pow5Factor(uint64_t value) {
  for (uint32_t count = 0; value > 0; count++) {
    if (value - 5 * (value / 5) != 0) {
      return count;
    }
    value /= 5;
  }
  return 0;
}

static inline bool multipleOfPowerOf5(uint64_t value, int32_t p) {
  return pow5Factor(value) >= p;
}
#endif // FAST_POW5

static inline uint32_t pow5bits(int32_t e) {
  return e == 0 ? 1 : (uint32_t) (((uint64_t) e * LOG2_5_NUMERATOR + LOG2_5_DENOMINATOR - 1) / LOG2_5_DENOMINATOR);
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
//    We explicitly case from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x86-64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsicts for the x86-64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bit of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#ifdef HAS_UINT128

// Best case: use 128-bit type.
static inline uint64_t mulPow5InvDivPow2(uint64_t m, int32_t i, int32_t j) {
  uint128_t b0 = ((uint128_t) m) * POW5_INV_SPLIT[i][0];
  uint128_t b2 = ((uint128_t) m) * POW5_INV_SPLIT[i][1];
  return (uint64_t) (((b0 >> 64) + b2) >> (j - 64));
}

static inline uint64_t mulPow5divPow2(uint64_t m, int32_t i, int32_t j) {
  uint128_t b0 = ((uint128_t) m) * POW5_SPLIT[i][0];
  uint128_t b2 = ((uint128_t) m) * POW5_SPLIT[i][1];
  return (uint64_t) (((b0 >> 64) + b2) >> (j - 64));
}

#else

#ifdef HAS_64_BIT_INTRINSICS

#include <intrin.h>
#pragma intrinsic(__umul128,__shiftright128)
#define umul128 __umul128
#define shiftright128 __shiftright128

#else // Use our own.

static inline uint64_t umul128(uint64_t a, uint64_t b, uint64_t* productHi) {
  uint64_t aLo = a & 0xffffffff;
  uint64_t aHi = a >> 32;
  uint64_t bLo = b & 0xffffffff;
  uint64_t bHi = b >> 32;

  uint64_t b00 = aLo * bLo;
  uint64_t b01 = aLo * bHi;
  uint64_t b10 = aHi * bLo;
  uint64_t b11 = aHi * bHi;

  uint64_t midSum = b01 + b10;
  uint64_t midCarry = midSum < b01;

  uint64_t productLo = b00 + (midSum << 32);
  uint64_t productLoCarry = productLo < b00;

  *productHi = b11 + (midSum >> 32) + (midCarry << 32) + productLoCarry;
  return productLo;
}

static inline uint64_t shiftright128(uint64_t lo, uint64_t hi, uint64_t dist) {
  // shift hi-lo right by 0 <= dist <= 128
  return (dist >= 64)
      ? hi >> (dist - 64)
      : (hi << (64 - dist)) + (lo >> dist);
}

#endif // HAS_64_BIT_INTRINSICS

static inline uint64_t mulPow5InvDivPow2(uint64_t m, int32_t i, int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                                             // 128
  uint64_t low1 = umul128(m, POW5_INV_SPLIT[i][1], &high1); // 64
  uint64_t high0;                                             // 64
  umul128(m, POW5_INV_SPLIT[i][0], &high0); // 0
  uint64_t sum = high0 + low1;
  if (sum < high0) high1++; // overflow into high1
  return shiftright128(sum, high1, j - 64);
}

static inline uint64_t mulPow5divPow2(uint64_t m, int32_t i, int32_t j) {
  // m is maximum 55 bits
  uint64_t high1;                                         // 128
  uint64_t low1 = umul128(m, POW5_SPLIT[i][1], &high1); // 64
  uint64_t high0;                                         // 64
  umul128(m, POW5_SPLIT[i][0], &high0); // 0
  uint64_t sum = high0 + low1;
  if (sum < high0) high1++; // overflow into high1
  return shiftright128(sum, high1, j - 64);
}

#endif // HAS_UINT128

static inline uint32_t decimalLength(uint64_t v) {
  // This is slightly faster than a loop. For a random set of numbers, the
  // average length is 17.4 digits, so we check high-to-low.
  if (v >= 1000000000000000000L) return 19;
  if (v >= 100000000000000000L) return 18;
  if (v >= 10000000000000000L) return 17;
  if (v >= 1000000000000000L) return 16;
  if (v >= 100000000000000L) return 15;
  if (v >= 10000000000000L) return 14;
  if (v >= 1000000000000L) return 13;
  if (v >= 100000000000L) return 12;
  if (v >= 10000000000L) return 11;
  if (v >= 1000000000L) return 10;
  if (v >= 100000000L) return 9;
  if (v >= 10000000L) return 8;
  if (v >= 1000000L) return 7;
  if (v >= 100000L) return 6;
  if (v >= 10000L) return 5;
  if (v >= 1000L) return 4;
  if (v >= 100L) return 3;
  if (v >= 10L) return 2;
  return 1;
}

void d2s_buffered(double f, char* result) {
  // Step 1: Decode the floating point number, and unify normalized and subnormal cases.
  int32_t mantissaBits = DOUBLE_MANTISSA_BITS;
  int32_t exponentBits = DOUBLE_EXPONENT_BITS;
  int32_t offset = (1 << (exponentBits - 1)) - 1;

  uint64_t bits = 0;
  // This only works on little-endian architectures.
  memcpy(&bits, &f, sizeof(double));

  // Decode bits into sign, mantissa, and exponent.
  bool sign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
  uint64_t ieeeMantissa = bits & ((1ull << mantissaBits) - 1);
  int32_t ieeeExponent = (int32_t) ((bits >> mantissaBits) & ((1 << exponentBits) - 1));

#ifdef DEBUG
  printf("IN=");
  for (int32_t bit = 63; bit >= 0; bit--) {
    printf("%d", (int) ((bits >> bit) & 1));
  }
  printf("\n");
#endif

  int32_t e2;
  uint64_t m2;
  // Case distinction; exit early for the easy cases.
  if (ieeeExponent == ((1 << exponentBits) - 1)) {
    strcpy(result, (ieeeMantissa != 0) ? "NaN" : sign ? "-Infinity" : "Infinity");
    return;
  } else if (ieeeExponent == 0) {
    if (ieeeMantissa == 0) {
      strcpy(result, sign ? "-0.0" : "0.0");
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

#ifdef DEBUG
  printf("S=%s E=%d M=%" PRIu64 "\n", sign ? "-" : "+", e2 + 2, m2);
#endif

  // Step 2: Determine the interval of legal decimal representations.
  uint64_t mv = 4 * m2;
  uint64_t mp = 4 * m2 + 2;
  // Implicit bool -> int conversion. True is 1, false is 0.
  uint64_t mm = 4 * m2 - 1 - ((m2 != (1ull << mantissaBits)) || (ieeeExponent <= 1));

  // Step 3: Convert to a decimal power base using 128-bit arithmetic.
  uint64_t vr;
#ifdef MATCH_GRISU3_OUTPUT
  bool vrIsTrailingZeros = false;
#endif
  uint64_t vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  if (e2 >= 0) {
    // I tried special-casing q == 0, but there was no effect on performance.
    int32_t q = max_uint32(0, ((int32_t) (((uint64_t) e2 * LOG10_2_NUMERATOR) / LOG10_2_DENOMINATOR)) - 1);
    e10 = q;
    int32_t k = POW5_INV_BITCOUNT + pow5bits(q) - 1;
    int32_t i = -e2 + q + k;
    vr = mulPow5InvDivPow2(mv, q, i);
    vp = mulPow5InvDivPow2(mp, q, i);
    vm = mulPow5InvDivPow2(mm, q, i);
#ifdef DEBUG
    printf("%" PRIu64 " * 2^%d / 10^%d\n", mv, e2, q);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 21) {
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
#ifdef MATCH_GRISU3_OUTPUT
        vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
#endif
      } else {
        if (acceptBounds) {
          // Same as min(e2 + (~mm & 1), pow5Factor(mm)) >= q
          // <=> e2 + (~mm & 1) >= q && pow5Factor(mm) >= q
          // <=> true && pow5Factor(mm) >= q, since e2 >= q.
          vmIsTrailingZeros = multipleOfPowerOf5(mm, q);
        } else {
          // Same as min(e2 + 1, pow5Factor(mp)) >= q.
          vp -= multipleOfPowerOf5(mp, q);
        }
      }
    }
  } else {
    int32_t q = max_uint32(0, ((int32_t) (((uint64_t) -e2 * LOG10_5_NUMERATOR) / LOG10_5_DENOMINATOR)) - 1);
    e10 = q + e2;
    int32_t i = -e2 - q;
    int32_t k = pow5bits(i) - POW5_BITCOUNT;
    int32_t j = q - k;
    vr = mulPow5divPow2(mv, i, j);
    vp = mulPow5divPow2(mp, i, j);
    vm = mulPow5divPow2(mm, i, j);
#ifdef DEBUG
    printf("%" PRIu64 " * 5^%d / 10^%d\n", mv, -e2, q);
    printf("%d %d %d %d\n", q, i, k, j);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 1) {
#ifdef MATCH_GRISU3_OUTPUT
      vrIsTrailingZeros = (mv & ((1ull << q) - 1)) == 0;
#endif
      if (acceptBounds) {
        vmIsTrailingZeros = (~mm & 1) >= q;
      } else {
        vp -= 1 >= q;
      }
    } else {
#ifdef MATCH_GRISU3_OUTPUT
      // We need to compute min(ntz(mv), pow5Factor(mv) - e2) >= q
      // <=> ntz(mv) >= q  &&  pow5Factor(mv) - e2 >= q
      // <=> ntz(mv) >= q
      // <=> mv & ((1 << q) - 1) == 0
      // We also need to make sure that the left shift does not overflow.
      vrIsTrailingZeros =
          ((q < mantissaBits) && ((mv & ((1ull << q) - 1)) == 0));
#if DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
#endif
    }
  }
#ifdef DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#ifdef MATCH_GRISU3_OUTPUT
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
#endif

  // Step 4: Find the shortest decimal representation in the interval of legal representations.
  int32_t vplength = decimalLength(vp);
  int32_t exp = e10 + vplength - 1;

  uint32_t removed = 0;
  int32_t lastRemovedDigit = 0;
  uint64_t output;
  // On average, we remove ~2 digits.
  if (vmIsTrailingZeros
#ifdef MATCH_GRISU3_OUTPUT
      || vrIsTrailingZeros
#endif
      ) {
    // General case, which happens rarely (<1%).
    while (vp / 10 > vm / 10) {
      // The compiler does not realize that vm % 10 can be computed from vm / 10
      // as vm - (vm / 10) * 10.
      vmIsTrailingZeros &= vm - (vm / 10) * 10 == 0; // vm % 10 == 0;
#ifdef MATCH_GRISU3_OUTPUT
      vrIsTrailingZeros &= lastRemovedDigit == 0;
#endif
      uint64_t nvr = vr / 10;
      lastRemovedDigit = vr - 10 * nvr;
      vr = nvr;
      vp /= 10;
      vm /= 10;
      removed++;
    }
#ifdef DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    // Same as above; use vm % 10 == vm - (vm / 10) * 10.
    if (vmIsTrailingZeros) {
      while (vm - (vm / 10) * 10 == 0) {
#ifdef MATCH_GRISU3_OUTPUT
        vrIsTrailingZeros &= lastRemovedDigit == 0;
#endif
        uint64_t nvr = vr / 10;
        lastRemovedDigit = vr - 10 * nvr;
        vr = nvr;
        vp /= 10;
        vm /= 10;
        removed++;
      }
    }
#ifdef DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
#ifdef MATCH_GRISU3_OUTPUT
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
#endif
#ifdef MATCH_GRISU3_OUTPUT
    if (vrIsTrailingZeros && (lastRemovedDigit == 5)) {
      if (vr % 10 != 7) {
        lastRemovedDigit = 4;
      }
    }
#endif
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr +
        ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || (lastRemovedDigit >= 5));
  } else {
    // Specialized for the common case (>99%).
    while (vp / 10 > vm / 10) {
      uint64_t nvr = vr / 10;
      lastRemovedDigit = vr - 10 * nvr;
      vr = nvr;
      vp /= 10;
      vm /= 10;
      removed++;
    }
#ifdef DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
#ifdef MATCH_GRISU3_OUTPUT
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
#endif
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm) || (lastRemovedDigit >= 5));
  }
  // The average output length is 16.38 digits.
  int32_t olength = vplength - removed;
#ifdef DEBUG
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
  int32_t i = 0;
  while (output >= 10000) {
    uint32_t c = output - 10000 * (output / 10000); // output % 10000;
    output /= 10000;
    uint32_t c0 = (c % 100) << 1;
    uint32_t c1 = (c / 100) << 1;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  while (output >= 100) {
    uint32_t c = (output - 100 * (output / 100)) << 1; // (output % 100) << 1;
    output /= 100;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (output >= 10) {
    uint32_t c = output << 1;
    result[index + olength - i] = DIGIT_TABLE[c + 1];
    result[index] = DIGIT_TABLE[c];
  } else {
    // Print the leading decimal digit.
    result[index] = '0' + output;
  }
#else
  // Print decimal digits after the decimal point.
  for (uint32_t i = 0; i < olength - 1; i++) {
    uint32_t c = output % 10; output /= 10;
    result[index + olength - i] = '0' + c;
  }
  // Print the leading decimal digit.
  result[index] = '0' + output % 10;
#endif // NO_DIGIT_TABLE

  // Print decimal point if needed.
  if (olength > 1) {
    result[index + 1] = '.';
    index += olength + 1;
  } else {
    index++;
  }

  // Print the exponent.
  result[index++] = 'E';
  if (exp < 0) {
    result[index++] = '-';
    exp = -exp;
  }

#ifndef NO_DIGIT_TABLE
  if (exp >= 100) {
    result[index++] = '0' + exp / 100;
    exp = exp - 100 * (exp / 100);
    memcpy(result + index, DIGIT_TABLE + (2 * exp), 2);
    index += 2;
  } else if (exp >= 10) {
    memcpy(result + index, DIGIT_TABLE + (2 * exp), 2);
    index += 2;
  } else {
    result[index++] = '0' + exp;
  }
#else
  if (exp >= 100) {
    result[index++] = '0' + exp / 100;
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
