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

#define POW10_ADDITIONAL_BITS 120
#define BITS (POW10_ADDITIONAL_BITS + 16)

#if defined(HAS_UINT128)

// Best case: use 128-bit type.
static inline uint32_t mulShift(const uint64_t m, const uint64_t* const mul, const int32_t j) {
  const uint128_t b0 = ((uint128_t) m) * mul[0]; // 0
  const uint128_t b1 = ((uint128_t) m) * mul[1]; // 64
  const uint128_t b2 = ((uint128_t) m) * mul[2]; // 128
  uint128_t s0 = b0 + (b1 << 64); // 0
  uint128_t c1 = s0 < b0;
  uint128_t s1 = b2 + (b1 >> 64) + c1; // 128
  if (j == 0) {
    assert(false);
  } else if (j < 128) {
    uint128_t r0 = (s0 >> j) | (s1 << (128 - j));
    uint128_t r1 = s1 >> j;
    uint128_t m0 = r1 % 1000000000;
    uint128_t m1 = ((m0 << 64) | (r0 >> 64)) % 1000000000;
    uint128_t m2 = ((m1 << 64) | (r0 & 0xffffffffffffffffull)) % 1000000000;
    return (uint32_t) m2;
  } else {
    assert(false);
  }
  return 0;
}

#elif defined(HAS_64_BIT_INTRINSICS)

#error "Not implemented"

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

static inline void append_nine_digits(uint32_t digits, char* const result) {
#ifdef RYU_DEBUG
  printf("DIGITS=%d\n", digits);
#endif

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

int d2fixed_buffered_n(double d, char* result) {
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
  if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u) || (ieeeExponent == 0 && ieeeMantissa == 0)) {
    return copy_special_str(result, ieeeSign, ieeeExponent, ieeeMantissa);
  }

  const uint32_t bias = (1u << (DOUBLE_EXPONENT_BITS - 1)) - 1;

  int32_t e2;
  uint64_t m2;
  if (ieeeExponent == 0) {
    e2 = 1 - bias - DOUBLE_MANTISSA_BITS;
    m2 = ieeeMantissa;
  } else {
    e2 = ieeeExponent - bias - DOUBLE_MANTISSA_BITS;
    m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  }

#ifdef RYU_DEBUG
  printf("-> %" PRIu64 " * 2^%d\n", m2, e2);
#endif

  uint32_t idx = indexForExponent(e2);
  uint32_t p10bits = pow10BitsForIndex(idx);
  uint32_t len = lengthForIndex(idx);
  int index = 0;
  bool nonzero = false;
  for (int i = len - 1; i >= 0; i--) {
    uint32_t j = p10bits - e2;
    uint32_t digits = mulShift(m2, POW10_SPLIT[POW10_OFFSET[idx] + i], j);
    if (nonzero) {
      append_nine_digits(digits, result + index);
      index += 9;
    } else if (digits != 0) {
      index += append_n_digits(digits, result + index);
      nonzero = true;
    }
  }
  return index;
}

char* d2fixed(double d) {
  char* buffer = malloc(2000);
  int index = d2fixed_buffered_n(d, buffer);
  buffer[index] = '\0';
  return buffer;
}