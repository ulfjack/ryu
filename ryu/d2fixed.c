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

#include "ryu/ryu2.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif

// ABSL avoids uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS)
#define HAS_UINT128
typedef __uint128_t uint128_t;
#elif defined(_MSC_VER) && !defined(RYU_ONLY_64_BIT_OPS) && defined(_M_X64) \
  && !defined(__clang__) // https://bugs.llvm.org/show_bug.cgi?id=37755
#define HAS_64_BIT_INTRINSICS
#endif

#include "ryu/common.h"
#include "ryu/digit_table.h"
#include "ryu/d2fixed_full_table.h"

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_BIAS 1023

#define POW10_ADDITIONAL_BITS 120
#define BITS (POW10_ADDITIONAL_BITS + 16)

#if defined(HAS_UINT128)

// Unfortunately, gcc/clang do not automatically turn a 128-bit integer division
// into a multiplication, so we have to do it manually.
static inline uint128_t mod10e9(uint128_t v) {
//  uint64_t m0 = (((uint64_t) (v >> 64)) % 1000000000) * 709551616;
//  uint64_t s1 = m0 + (uint64_t) v;
//  if (s1 < m0) {
//    return (s1 % 1000000000 + 709551616) % 1000000000;
//  } else {
//    return s1 % 1000000000;
//  }

  uint64_t m0 = ((uint64_t) (v >> 64)) % 1000000000;
  uint64_t m1 = ((uint64_t) v) % 1000000000;
  // 2^64 % 1000000000 = 709551616
  uint64_t m2 = (m0 * 709551616 + m1) % 1000000000;
  return m2;
}

// Best case: use 128-bit type.
static inline uint32_t mulShift(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  const uint128_t b0 = ((uint128_t) m) * mul[0]; // 0
  const uint128_t b1 = ((uint128_t) m) * mul[1]; // 64
  const uint128_t b2 = ((uint128_t) m) * mul[2]; // 128
  if (j <= 64) {
    assert(false);
  } else if (j < 128) {
    uint64_t b1lo = (uint64_t) b1;
    uint64_t s0 = b1lo + (b0 >> 64); // 64
    uint128_t c1 = s0 < b1lo;
    uint128_t s1 = b2 + (b1 >> 64) + c1; // 128
    uint128_t r0 = (mod10e9(s1) << 64) + s0;
    return (uint32_t) mod10e9(r0 >> (j - 64));
  } else if (j == 128) {
    uint128_t s0 = b0 + (b1 << 64); // 0
    uint128_t c1 = s0 < b0;
    uint128_t s1 = b2 + (b1 >> 64) + c1; // 128
    return (uint32_t) mod10e9(s1);
  } else if (j < 256) {
    uint128_t s0 = b0 + (b1 << 64); // 0
    uint128_t c1 = s0 < b0;
    uint128_t s1 = b2 + (b1 >> 64) + c1; // 128
    return (uint32_t) mod10e9(s1 >> (j - 128));
  }
  return 0;
}

static inline uint32_t mulShift2(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  const uint128_t b0 = ((uint128_t) m) * mul[0]; // 0
  const uint128_t b1 = ((uint128_t) m) * mul[1]; // 64
  if (j <= 64) {
    assert(false);
  } else if (j < 192) { // 64 < j < 192
    uint128_t s = (b0 >> 64) + b1; // 64
    return (uint32_t) mod10e9(s >> (j - 64));
  }
  return 0;
}

#elif defined(HAS_64_BIT_INTRINSICS)

#include <intrin.h>

static inline uint64_t umul128(const uint64_t a, const uint64_t b, uint64_t* const productHi) {
  return _umul128(a, b, productHi);
}

static inline uint32_t mulShift(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  uint64_t high0;                                   // 64
  const uint64_t low0 = umul128(m, mul[0], &high0); // 0
  uint64_t high1;                                   // 128
  const uint64_t low1 = umul128(m, mul[1], &high1); // 64
  uint64_t high2;                                   // 192
  const uint64_t low2 = umul128(m, mul[2], &high2); // 128
  const uint64_t s0low = low0;              // 0
  const uint64_t s0high = high0 + low1;     // 64
  const uint64_t c1 = s0high < high0;
  const uint64_t s1low = high1 + low2 + c1; // 128
  const uint64_t c2 = s1low < high1;
  const uint64_t s1high = high2 + c2;       // 192
  if (j < 128) {
    assert(false);
  } else if (j == 128) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff));
    return r2 % 1000000000;
  } else if (j < 160) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff));
    return (r2 >> (j - 128)) % 1000000000;
  } else if (j == 160) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32));
    return r1 % 1000000000;
  } else if (j < 192) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32));
    return (r1 >> (j - 160)) % 1000000000;
  } else if (j == 192) {
    return s1high % 1000000000;
  } else if (j < 256) {
    return (s1high << (j - 192)) % 1000000000;
  }
  return 0;
}

static inline uint32_t mulShift2(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  uint64_t high0;                                   // 64
  const uint64_t low0 = umul128(m, mul[0], &high0); // 0
  uint64_t high1;                                   // 128
  const uint64_t low1 = umul128(m, mul[1], &high1); // 64
  const uint64_t high2 = 0;                         // 192
  const uint64_t low2 = 0;                          // 128
  const uint64_t s0low = low0;              // 0
  const uint64_t s0high = high0 + low1;     // 64
  const uint64_t c1 = s0high < high0;
  const uint64_t s1low = high1 + low2 + c1; // 128
  const uint64_t c2 = s1low < high1;
  const uint64_t s1high = high2 + c2;       // 192
  if (j < 64) {
    printf("%d\n", j);
    assert(false);
  } else if (j < 96) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff)) % 1000000000;
    const uint64_t r3 = ((r2 << 32) | (s0high >> 32)) % 1000000000;
    const uint64_t r4 = ((r3 << 32) | (s0high & 0xffffffff));
    return (r4 >> (j - 64)) % 1000000000;
  } else if (j == 96) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff)) % 1000000000;
    const uint64_t r3 = ((r2 << 32) | (s0high >> 32)) % 1000000000;
    return r3 % 1000000000;
  } else if (j < 128) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff)) % 1000000000;
    const uint64_t r3 = ((r2 << 32) | (s0high >> 32)) % 1000000000;
    return (r3 >> (j - 64)) % 1000000000;
  } else if (j == 128) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff));
    return r2 % 1000000000;
  } else if (j < 160) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32)) % 1000000000;
    const uint64_t r2 = ((r1 << 32) | (s1low & 0xffffffff));
    return (r2 >> (j - 128)) % 1000000000;
  } else if (j == 160) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32));
    return r1 % 1000000000;
  } else if (j < 192) {
    const uint64_t r0 = s1high % 1000000000;
    const uint64_t r1 = ((r0 << 32) | (s1low >> 32));
    return (r1 >> (j - 160)) % 1000000000;
  } else if (j == 192) {
    return s1high % 1000000000;
  } else if (j < 256) {
    return (s1high << (j - 192)) % 1000000000;
  }
  return 0;
}

#else // !defined(HAS_UINT128) && !defined(HAS_64_BIT_INTRINSICS)

#error "Not implemented"

#endif // HAS_64_BIT_INTRINSICS

static inline uint32_t decimalLength(const uint32_t v) {
  assert(v < 1000000000L);
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

static inline uint32_t append_n_digits(uint32_t digits, char* const result) {
#ifdef RYU_DEBUG
  printf("DIGITS=%d\n", digits);
#endif

  uint32_t olength = decimalLength(digits);
  uint32_t i = 0;
  while (digits >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
    const uint32_t c = digits - 10000 * (digits / 10000);
#else
    const uint32_t c = digits % 10000;
#endif
    digits /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    memcpy(result + olength - i - 2, DIGIT_TABLE + c0, 2);
    memcpy(result + olength - i - 4, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (digits >= 100) {
    const uint32_t c = (digits % 100) << 1;
    digits /= 100;
    memcpy(result + olength - i - 2, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (digits >= 10) {
    const uint32_t c = digits << 1;
    memcpy(result + olength - i - 2, DIGIT_TABLE + c, 2);
  } else {
    result[0] = (char) ('0' + digits);
  }
  return olength;
}

static inline void append_d_digits(uint32_t olength, uint32_t digits, char* const result) {
#ifdef RYU_DEBUG
  printf("DIGITS=%d\n", digits);
#endif

  uint32_t i = 0;
  while (digits >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
    const uint32_t c = digits - 10000 * (digits / 10000);
#else
    const uint32_t c = digits % 10000;
#endif
    digits /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    memcpy(result + olength + 1 - i - 2, DIGIT_TABLE + c0, 2);
    memcpy(result + olength + 1 - i - 4, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (digits >= 100) {
    const uint32_t c = (digits % 100) << 1;
    digits /= 100;
    memcpy(result + olength + 1 - i - 2, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (digits >= 10) {
    result[2] = (char) ('0' + digits % 10);
    result[1] = '.';
    result[0] = (char) ('0' + digits / 10);
  } else {
    result[1] = '.';
    result[0] = (char) ('0' + digits);
  }
}

static inline void append_c_digits(uint32_t count, uint32_t digits, char* const result) {
#ifdef RYU_DEBUG
  printf("DIGITS=%d\n", digits);
#endif
  int i = 0;
  for (; i < count - 1; i += 2) {
    const uint32_t c = (digits % 100) << 1;
    digits /= 100;
    memcpy(result + count - i - 2, DIGIT_TABLE + c, 2);
  }
  if (i < count) {
    char c = '0' + (digits % 10);
    digits /= 10;
    result[count - i - 1] = c;
  }
}

static inline void append_nine_digits(uint32_t digits, char* const result) {
#ifdef RYU_DEBUG
  printf("DIGITS=%d\n", digits);
#endif
  if (digits == 0) {
    memset(result, '0', 9);
    return;
  }

  for (uint32_t i = 0; i < 5; i += 4) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
    const uint32_t c = digits - 10000 * (digits / 10000);
#else
    const uint32_t c = digits % 10000;
#endif
    digits /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    memcpy(result + 7 - i, DIGIT_TABLE + c0, 2);
    memcpy(result + 5 - i, DIGIT_TABLE + c1, 2);
  }
  result[0] = (char) ('0' + digits);
}

static uint32_t indexForExponent(uint32_t e) {
  return (e + 15) / 16;
}

static uint32_t pow10BitsForIndex(uint32_t idx) {
  return 16 * idx + POW10_ADDITIONAL_BITS;
}

static uint32_t lengthForIndex(uint32_t idx) {
  // [log_10(2^i)] = ((16 * i) * 78913L) >> 18 <-- floor
  // +1 for ceil, +16 for mantissa, +8 to round up when dividing by 9
  return ((uint32_t) (((16 * idx) * 1292913986ull) >> 32) + 1 + 16 + 8) / 9;
}

static inline uint32_t pow5Factor(uint64_t value) {
  uint32_t count = 0;
  for (;;) {
    assert(value != 0);
    const uint64_t q = value / 5;
    const uint32_t r = (uint32_t) (value - 5 * q);
    if (r != 0) {
      break;
    }
    value = q;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
static inline bool multipleOfPowerOf5(const uint64_t value, const uint32_t p) {
  // I tried a case distinction on p, but there was no performance difference.
  return pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
static inline bool multipleOfPowerOf2(const uint64_t value, const uint32_t p) {
  assert(value != 0);
  // return __builtin_ctzll(value) >= p;
  return (value & ((1ull << p) - 1)) == 0;
}

static inline int copy_special_str_printf(char * const result, const bool sign, const bool exponent, const uint64_t mantissa) {
  if (sign) {
    result[0] = '-';
  }
  if (mantissa) {
#if defined(_MSC_VER)
    if (mantissa < (1ull << (DOUBLE_MANTISSA_BITS - 1))) {
      memcpy(result + sign, "nan(snan)", 9);
      return sign + 9;
    }
#endif
    memcpy(result + sign, "nan", 3);
    return sign + 3;
  }
  if (exponent) {
    memcpy(result + sign, "Infinity", 8);
    return sign + 8;
  }
  memcpy(result + sign, "0", 1);
  return sign + 1;
}

int d2fixed_buffered_n(double d, uint32_t precision, char* result) {
  const uint64_t bits = double_to_bits(d);
#ifdef RYU_DEBUG
  printf("IN=");
  for (int32_t bit = 63; bit >= 0; --bit) {
    printf("%d", (int) ((bits >> bit) & 1));
  }
  printf("\n");
#endif

  // Decode bits into sign, mantissa, and exponent.
  const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1) != 0;
  const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
  const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));

  // Case distinction; exit early for the easy cases.
  if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u)) {
    return copy_special_str_printf(result, ieeeSign, ieeeExponent, ieeeMantissa);
  }
  if (ieeeExponent == 0 && ieeeMantissa == 0) {
    int index = 0;
    if (ieeeSign) {
      result[index++] = '-';
    }
    result[index++] = '0';
    result[index++] = '.';
    memset(result + index, '0', precision);
    index += precision;
    return index;
  }

  int32_t e2;
  uint64_t m2;
  if (ieeeExponent == 0) {
    e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
    m2 = ieeeMantissa;
  } else {
    e2 = ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
    m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  }

#ifdef RYU_DEBUG
  printf("-> %" PRIu64 " * 2^%d\n", m2, e2);
#endif

  int index = 0;
  bool nonzero = false;
  if (ieeeSign) {
    result[index++] = '-';
  }
  if (e2 >= -53) {
    int32_t idx = e2 < 0 ? 0 : indexForExponent(e2);
    int32_t p10bits = pow10BitsForIndex(idx);
    int32_t len = lengthForIndex(idx);
#ifdef RYU_DEBUG
    printf("idx=%d\n", idx);
    printf("len=%d\n", len);
#endif
    for (int i = len - 1; i >= 0; i--) {
      uint32_t j = p10bits - e2;
      // Temporary: j is usually around 128, and by shifting a bit, we push it above 128, which is
      // a slightly faster code path in mulShift. Instead, we can just increase the multipliers.
      uint32_t digits = mulShift(m2 << 8, POW10_SPLIT[POW10_OFFSET[idx] + i], j + 8);
      if (nonzero) {
        append_nine_digits(digits, result + index);
        index += 9;
      } else if (digits != 0) {
        index += append_n_digits(digits, result + index);
        nonzero = true;
      }
    }
  }
  if (!nonzero) {
    result[index++] = '0';
  }
  result[index++] = '.';
#ifdef RYU_DEBUG
  printf("e2=%d\n", e2);
#endif
  if (e2 < 0) {
    int32_t idx = -e2 / 16;
#ifdef RYU_DEBUG
    printf("idx=%d\n", idx);
#endif
    int32_t blocks = precision / 9 + 1;
    // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
    int roundUp = 0;
    int32_t i = 0;
    if (i < MIN_BLOCK_2[idx]) {
      i = blocks - 1 < MIN_BLOCK_2[idx] ? blocks - 1 : MIN_BLOCK_2[idx];
      memset(result + index, '0', 9 * i);
      index += 9 * i;
    }
    for (; i < blocks; i++) {
      int32_t j = ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
      int32_t p = POW10_OFFSET_2[idx] + i;
      if (p >= POW10_OFFSET_2[idx + 1]) {
        // If the remaining digits are all 0, then we might as well use memset.
        // No rounding required in this case.
        int32_t fill = precision - 9 * i;
        memset(result + index, '0', fill);
        index += fill;
        break;
      }
      uint32_t digits = mulShift2(m2, POW10_SPLIT_2[p], j);
#ifdef RYU_DEBUG
      printf("digits=%u\n", digits);
      printf("idx=%d\n", idx);
#endif
      if (i < blocks - 1) {
        append_nine_digits(digits, result + index);
        index += 9;
      } else {
        int32_t max = precision - 9 * i;
        int32_t lastDigit = 0;
        for (int32_t k = 0; k < 9 - max; k++) {
          lastDigit = digits % 10;
          digits /= 10;
        }
#ifdef RYU_DEBUG
        printf("lastDigit=%d\n", lastDigit);
#endif
        if (lastDigit != 5) {
          roundUp = lastDigit > 5;
        } else {
          // Is m * 10^(additionalDigits + 1) / 2^(-e2) integer?
          int32_t requiredTwos = -e2 - precision - 1;
          bool trailingZeros = (requiredTwos <= 0) || (requiredTwos < 60 && multipleOfPowerOf2(m2, requiredTwos));
          roundUp = trailingZeros ? 2 : 1;
#ifdef RYU_DEBUG
          printf("requiredTwos=%d\n", requiredTwos);
          printf("trailingZeros=%s\n", trailingZeros ? "true" : "false");
#endif
        }
        if (max > 0) {
          append_c_digits(max, digits, result + index);
          index += max;
        }
        break;
      }
    }
#ifdef RYU_DEBUG
    printf("roundUp=%d\n", roundUp);
#endif
    if (roundUp != 0) {
      int roundIndex = index;
      while (true) {
        roundIndex--;
        char c = result[roundIndex];
        if (c == '.') {
          continue;
        } else if (c == '9') {
          result[roundIndex] = '0';
          roundUp = 1;
          continue;
        } else {
          if ((roundUp == 2) && (c % 2 == 0)) {
            break;
          }
          result[roundIndex] = c + 1;
          break;
        }
      }
    }
  } else {
    memset(result + index, '0', precision);
    index += precision;
  }
  return index;
}

void d2fixed_buffered(double d, uint32_t precision, char* result) {
  int len = d2fixed_buffered_n(d, precision, result);
  result[len] = '\0';
}

char* d2fixed(double d, uint32_t precision) {
  char* buffer = malloc(2000);
  int index = d2fixed_buffered_n(d, precision, buffer);
  buffer[index] = '\0';
  return buffer;
}



int d2exp_buffered_n(double d, uint32_t precision, char* result) {
  const uint64_t bits = double_to_bits(d);
#ifdef RYU_DEBUG
  printf("IN=");
  for (int32_t bit = 63; bit >= 0; --bit) {
    printf("%d", (int) ((bits >> bit) & 1));
  }
  printf("\n");
#endif

  // Decode bits into sign, mantissa, and exponent.
  const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1) != 0;
  const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
  const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));

  // Case distinction; exit early for the easy cases.
  if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u)) {
    return copy_special_str_printf(result, ieeeSign, ieeeExponent, ieeeMantissa);
  }
  if (ieeeExponent == 0 && ieeeMantissa == 0) {
    int index = 0;
    if (ieeeSign) {
      result[index++] = '-';
    }
    result[index++] = '0';
    result[index++] = '.';
    memset(result + index, '0', precision);
    index += precision;
    return index;
  }

  int32_t e2;
  uint64_t m2;
  if (ieeeExponent == 0) {
    e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
    m2 = ieeeMantissa;
  } else {
    e2 = ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
    m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  }

#ifdef RYU_DEBUG
  printf("-> %" PRIu64 " * 2^%d\n", m2, e2);
#endif

  precision++;
  int index = 0;
  if (ieeeSign) {
    result[index++] = '-';
  }
  uint32_t digits = 0;
  uint32_t printedDigits = 0;
  uint32_t availableDigits = 0;
  int32_t exp = 0;
  if (e2 >= -52) {
    int32_t idx = e2 < 0 ? 0 : indexForExponent(e2);
    int32_t p10bits = pow10BitsForIndex(idx);
    int32_t len = lengthForIndex(idx);
#ifdef RYU_DEBUG
    printf("idx=%d\n", idx);
    printf("len=%d\n", len);
#endif
    for (int i = len - 1; i >= 0; i--) {
      uint32_t j = p10bits - e2;
      // Temporary: j is usually around 128, and by shifting a bit, we push it above 128, which is
      // a slightly faster code path in mulShift. Instead, we can just increase the multipliers.
      digits = mulShift(m2 << 8, POW10_SPLIT[POW10_OFFSET[idx] + i], j + 8);
      if (printedDigits != 0) {
        if (printedDigits + 9 > precision) {
          availableDigits = 9;
          break;
        }
        append_nine_digits(digits, result + index);
        index += 9;
        printedDigits += 9;
      } else if (digits != 0) {
        availableDigits = decimalLength(digits);
        exp = i * 9 + availableDigits - 1;
        if (printedDigits + availableDigits > precision) {
          break;
        }
        append_d_digits(availableDigits, digits, result + index);
        index += availableDigits + 1;
        printedDigits += availableDigits;
        availableDigits = 0;
      }
    }
  }

  if (e2 < 0 && availableDigits == 0) {
    int32_t idx = -e2 / 16;
#ifdef RYU_DEBUG
    printf("e2=%d\n", e2);
    printf("idx=%d\n", idx);
#endif
    for (int32_t i = MIN_BLOCK_2[idx]; i < 200; i++) {
      int32_t j = ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
      int32_t p = POW10_OFFSET_2[idx] + i;
      digits = (p >= POW10_OFFSET_2[idx + 1]) ? 0 : mulShift2(m2, POW10_SPLIT_2[p], j);
#ifdef RYU_DEBUG
      printf("digits=%d\n", e2);
      printf("idx=%d\n", idx);
#endif
      if (printedDigits != 0) {
        if (printedDigits + 9 > precision) {
          availableDigits = 9;
          break;
        }
        append_nine_digits(digits, result + index);
        index += 9;
        printedDigits += 9;
      } else if (digits != 0) {
        availableDigits = decimalLength(digits);
        exp = -(i + 1) * 9 + availableDigits - 1;
        if (printedDigits + availableDigits > precision) {
          break;
        }
        append_d_digits(availableDigits, digits, result + index);
        index += availableDigits + 1;
        printedDigits += availableDigits;
        availableDigits = 0;
      }
    }
  }

  int32_t max = precision - printedDigits;
#ifdef RYU_DEBUG
  printf("availableDigits=%d\n", availableDigits);
  printf("digits=%d\n", digits);
  printf("max=%d\n", max);
#endif
  if (availableDigits == 0) {
    digits = 0;
  }
  int32_t lastDigit = 0;
  if (availableDigits > max) {
    for (int32_t k = 0; k < availableDigits - max; k++) {
      lastDigit = digits % 10;
      digits /= 10;
    }
  }
#ifdef RYU_DEBUG
  printf("lastDigit=%d\n", lastDigit);
#endif
  // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
  int roundUp = 0;
  if (lastDigit != 5) {
    roundUp = lastDigit > 5;
  } else {
    // Is m * 2^e2 * 10^(precision + 1 - exp) integer?
    bool trailingZeros;
    // precision was already increased by 1, so we don't need to write + 1 here.
    int32_t rexp = precision - exp;
    if (rexp < 0) {
      int32_t requiredTwos = -e2 - rexp;
      int32_t requiredFives = -rexp;
      trailingZeros =
          (requiredTwos <= 0 || (requiredTwos < 60 && multipleOfPowerOf2(m2, requiredTwos))) && multipleOfPowerOf5(m2, requiredFives);
    } else {
      int32_t requiredTwos = -e2 - rexp;
      trailingZeros = requiredTwos <= 0 || (requiredTwos < 60 && multipleOfPowerOf2(m2, requiredTwos));
    }
    roundUp = trailingZeros ? 2 : 1;
#ifdef RYU_DEBUG
//    printf("requiredTwos=%d\n", requiredTwos);
    printf("trailingZeros=%s\n", trailingZeros ? "true" : "false");
#endif
  }
  if (printedDigits != 0) {
    if (digits == 0) {
      memset(result + index, '0', max);
    } else {
      append_c_digits(max, digits, result + index);
    }
    index += max;
  } else {
    append_d_digits(max, digits, result + index);
    index += max + 1;
  }
#ifdef RYU_DEBUG
  printf("roundUp=%d\n", roundUp);
#endif
  if (roundUp != 0) {
    int roundIndex = index;
    while (true) {
      roundIndex--;
      char c = result[roundIndex];
      if (roundIndex == -1 || c == '-') {
        result[roundIndex + 1] = '1';
        exp++;
        break;
      }
      if (c == '.') {
        continue;
      } else if (c == '9') {
        result[roundIndex] = '0';
        roundUp = 1;
        continue;
      } else {
        if ((roundUp == 2) && (c % 2 == 0)) {
          break;
        }
        result[roundIndex] = c + 1;
        break;
      }
    }
  }
  result[index++] = 'e';
  if (exp < 0) {
    result[index++] = '-';
    exp = -exp;
  } else {
    result[index++] = '+';
  }
  if (exp < 10) {
    result[index++] = '0';
  }
  index += append_n_digits(exp, result + index);
  return index;
}

void d2exp_buffered(double d, uint32_t precision, char* result) {
  int len = d2exp_buffered_n(d, precision, result);
  result[len] = '\0';
}

char* d2exp(double d, uint32_t precision) {
  char* buffer = malloc(2000);
  int index = d2exp_buffered_n(d, precision, buffer);
  buffer[index] = '\0';
  return buffer;
}
