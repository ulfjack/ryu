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

#include "ryu/ryu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_DIGIT_TABLE
#include "ryu/digit_table.h"
#endif

#ifdef RYU_DEBUG
#include <stdio.h>
#endif

#define FLOAT_MANTISSA_BITS 23
#define FLOAT_EXPONENT_BITS 8

// These computations using these constants are only safe for the ranges
// required by 32-bit floating-point numbers.
#define FLOAT_LOG10_2_DENOMINATOR 10000000u
#define FLOAT_LOG10_2_NUMERATOR 3010299u // FLOAT_LOG10_2_DENOMINATOR * log_10(2)
#define FLOAT_LOG10_5_DENOMINATOR 10000000u
#define FLOAT_LOG10_5_NUMERATOR 6989700u // FLOAT_LOG10_5_DENOMINATOR * log_10(5)
#define FLOAT_LOG2_5_DENOMINATOR 10000000u
#define FLOAT_LOG2_5_NUMERATOR 23219280u // FLOAT_LOG2_5_DENOMINATOR * log_2(5)

// This table is generated by PrintFloatLookupTable.
#define FLOAT_POW5_INV_BITCOUNT 59
static const uint64_t FLOAT_POW5_INV_SPLIT[31] = {
  576460752303423489u, 461168601842738791u, 368934881474191033u, 295147905179352826u,
  472236648286964522u, 377789318629571618u, 302231454903657294u, 483570327845851670u,
  386856262276681336u, 309485009821345069u, 495176015714152110u, 396140812571321688u,
  316912650057057351u, 507060240091291761u, 405648192073033409u, 324518553658426727u,
  519229685853482763u, 415383748682786211u, 332306998946228969u, 531691198313966350u,
  425352958651173080u, 340282366920938464u, 544451787073501542u, 435561429658801234u,
  348449143727040987u, 557518629963265579u, 446014903970612463u, 356811923176489971u,
  570899077082383953u, 456719261665907162u, 365375409332725730u
};
#define FLOAT_POW5_BITCOUNT 61
static const uint64_t FLOAT_POW5_SPLIT[47] = {
 1152921504606846976u, 1441151880758558720u, 1801439850948198400u, 2251799813685248000u,
 1407374883553280000u, 1759218604441600000u, 2199023255552000000u, 1374389534720000000u,
 1717986918400000000u, 2147483648000000000u, 1342177280000000000u, 1677721600000000000u,
 2097152000000000000u, 1310720000000000000u, 1638400000000000000u, 2048000000000000000u,
 1280000000000000000u, 1600000000000000000u, 2000000000000000000u, 1250000000000000000u,
 1562500000000000000u, 1953125000000000000u, 1220703125000000000u, 1525878906250000000u,
 1907348632812500000u, 1192092895507812500u, 1490116119384765625u, 1862645149230957031u,
 1164153218269348144u, 1455191522836685180u, 1818989403545856475u, 2273736754432320594u,
 1421085471520200371u, 1776356839400250464u, 2220446049250313080u, 1387778780781445675u,
 1734723475976807094u, 2168404344971008868u, 1355252715606880542u, 1694065894508600678u,
 2117582368135750847u, 1323488980084844279u, 1654361225106055349u, 2067951531382569187u,
 1292469707114105741u, 1615587133892632177u, 2019483917365790221u
};

static inline int32_t pow5Factor(uint32_t value) {
  for (int32_t count = 0; value > 0; ++count) {
    if (value - 5 * (value / 5) != 0) {
      return count;
    }
    value /= 5;
  }
  return 0;
}

// Returns true if value divides 5^p.
static inline bool multipleOfPowerOf5(uint32_t value, int32_t p) {
  return pow5Factor(value) >= p;
}

static inline uint32_t float_pow5bits(int32_t e) {
  return e == 0
      ? 1
      // We need to round up in this case.
      : (uint32_t) ((e * FLOAT_LOG2_5_NUMERATOR + FLOAT_LOG2_5_DENOMINATOR - 1) / FLOAT_LOG2_5_DENOMINATOR);
}

// It seems to be slightly faster to avoid uint128_t here, although the
// generated code for uint128_t looks slightly nicer.
static inline uint64_t mulPow5InvDivPow2(uint32_t m, uint32_t q, int32_t j) {
  uint64_t factor = FLOAT_POW5_INV_SPLIT[q];
  uint64_t bits0 = m * (factor & 0xffffffffu);
  uint64_t bits1 = m * (factor >> 32);
  return ((bits0 >> 32) + bits1) >> (j - 32);
}

static inline uint64_t mulPow5divPow2(uint32_t m, uint32_t i, int32_t j) {
  uint64_t factor = FLOAT_POW5_SPLIT[i];
  uint64_t bits0 = m * (factor & 0xffffffffu);
  uint64_t bits1 = m * (factor >> 32);
  return ((bits0 >> 32) + bits1) >> (j - 32);
}

static inline uint32_t decimalLength(uint32_t v) {
  if (v >= 1000000000) { return 10; }
  if (v >= 100000000) { return 9; }
  if (v >= 10000000) { return 8; }
  if (v >= 1000000) { return 7; }
  if (v >= 100000) { return 6; }
  if (v >= 10000) { return 5; }
  if (v >= 1000) { return 4; }
  if (v >= 100) { return 3; }
  if (v >= 10) { return 2; }
  return 1;
}

void f2s_buffered(float f, char* result) {
  // Step 1: Decode the floating-point number, and unify normalized and subnormal cases.
  uint32_t mantissaBits = FLOAT_MANTISSA_BITS;
  uint32_t exponentBits = FLOAT_EXPONENT_BITS;
  uint32_t offset = (1u << (exponentBits - 1)) - 1;

  uint32_t bits = 0;
  // This only works on little-endian architectures.
  memcpy(&bits, &f, sizeof(float));

  // Decode bits into sign, mantissa, and exponent.
  bool sign = ((bits >> (mantissaBits + exponentBits)) & 1) != 0;
  uint32_t ieeeMantissa = bits & ((1u << mantissaBits) - 1);
  uint32_t ieeeExponent = (uint32_t) ((bits >> mantissaBits) & ((1u << exponentBits) - 1));

#ifdef RYU_DEBUG
  printf("IN=");
  for (int32_t bit = 31; bit >= 0; --bit) {
    printf("%d", (bits >> bit) & 1);
  }
  printf("\n");
#endif

  int32_t e2;
  uint32_t m2;
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
    m2 = (1u << mantissaBits) | ieeeMantissa;
  }
  bool even = (m2 & 1) == 0;
  bool acceptBounds = even;

#ifdef RYU_DEBUG
  printf("S=%s E=%d M=%u\n", sign ? "-" : "+", e2, m2);
#endif

  // Step 2: Determine the interval of legal decimal representations.
  uint32_t mv = 4 * m2;
  uint32_t mp = 4 * m2 + 2;
  uint32_t mm = 4 * m2 - (((m2 != (1u << mantissaBits)) || (ieeeExponent <= 1)) ? 2 : 1);

  // Step 3: Convert to a decimal power base using 64-bit arithmetic.
  uint32_t vr;
  uint32_t vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  uint8_t lastRemovedDigit = 0;
  if (e2 >= 0) {
    int32_t q = (int32_t) ((e2 * FLOAT_LOG10_2_NUMERATOR) / FLOAT_LOG10_2_DENOMINATOR);
    e10 = q;
    int32_t k = FLOAT_POW5_INV_BITCOUNT + float_pow5bits(q) - 1;
    int32_t i = -e2 + q + k;
    vr = (uint32_t) mulPow5InvDivPow2(mv, q, i);
    vp = (uint32_t) mulPow5InvDivPow2(mp, q, i);
    vm = (uint32_t) mulPow5InvDivPow2(mm, q, i);
#ifdef RYU_DEBUG
    printf("%d * 2^%d / 10^%d\n", mv, e2, q);
    printf("V+=%d\nV =%d\nV-=%d\n", vp, vr, vm);
#endif
    if (q != 0 && ((vp - 1) / 10 <= vm / 10)) {
      // We need to know one removed digit even if we are not going to loop below. We could use
      // q = X - 1 above, except that would require 33 bits for the result, and we've found that
      // 32-bit arithmetic is faster even on 64-bit machines.
      int32_t l = FLOAT_POW5_INV_BITCOUNT + float_pow5bits(q - 1) - 1;
      lastRemovedDigit = (uint8_t) (mulPow5InvDivPow2(mv, q - 1, -e2 + q - 1 + l) % 10);
    }
    if (q <= 9) {
      // Only one of mp, mv, and mm can be a multiple of 5, if any.
      if (mv % 5 == 0) {
        vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
      } else {
        if (acceptBounds) {
          vmIsTrailingZeros = multipleOfPowerOf5(mm, q);
        } else {
          vp -= multipleOfPowerOf5(mp, q);
        }
      }
    }
  } else {
    int32_t q = (int32_t) ((-e2 * FLOAT_LOG10_5_NUMERATOR) / FLOAT_LOG10_5_DENOMINATOR);
    e10 = q + e2;
    int32_t i = -e2 - q;
    int32_t k = float_pow5bits(i) - FLOAT_POW5_BITCOUNT;
    int32_t j = q - k;
    vr = (uint32_t) mulPow5divPow2(mv, i, j);
    vp = (uint32_t) mulPow5divPow2(mp, i, j);
    vm = (uint32_t) mulPow5divPow2(mm, i, j);
#ifdef RYU_DEBUG
    printf("%d * 5^%d / 10^%d\n", mv, -e2, q);
    printf("%d %d %d %d\n", q, i, k, j);
    printf("V+=%d\nV =%d\nV-=%d\n", vp, vr, vm);
#endif
    if (q != 0 && ((vp - 1) / 10 <= vm / 10)) {
      j = q - 1 - (float_pow5bits(i + 1) - FLOAT_POW5_BITCOUNT);
      lastRemovedDigit = (uint8_t) (mulPow5divPow2(mv, i + 1, j) % 10);
    }
    if (q <= 1) {
      vrIsTrailingZeros = (~mv & 1) >= (uint32_t) q;
      if (acceptBounds) {
        vmIsTrailingZeros = (~mm & 1) >= (uint32_t) q;
      } else {
        --vp;
      }
    } else if (q < 31) { // TODO(ulfjack): Use a tighter bound here.
      vrIsTrailingZeros = (mv & ((1u << (q - 1)) - 1)) == 0;
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%u\nV =%u\nV-=%u\n", vp, vr, vm);
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif

  // Step 4: Find the shortest decimal representation in the interval of legal representations.
  uint32_t vplength = decimalLength(vp);
  int32_t exp = e10 + vplength - 1;

  uint32_t removed = 0;
  uint32_t output;
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    // General case, which happens rarely.
    while (vp / 10 > vm / 10) {
      vmIsTrailingZeros &= vm % 10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      uint64_t nvr = vr / 10;
      lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
      vr = (uint32_t) nvr;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
    if (vmIsTrailingZeros) {
      while (vm % 10 == 0) {
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        uint64_t nvr = vr / 10;
        lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
        vr = (uint32_t) nvr;
        vp /= 10;
        vm /= 10;
        ++removed;
      }
    }
    if (vrIsTrailingZeros && (lastRemovedDigit == 5) && (vr % 2 == 0)) {
      // Round down not up if the number ends in X50000.
      lastRemovedDigit = 4;
    }
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr +
        ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || (lastRemovedDigit >= 5));
  } else {
    // Common case.
    while (vp / 10 > vm / 10) {
      uint64_t nvr = vr / 10;
      lastRemovedDigit = (uint8_t) (vr - 10 * nvr);
      vr = (uint32_t) nvr;
      vp /= 10;
      vm /= 10;
      ++removed;
    }
    // We need to take vr+1 if vr is outside bounds or we need to round up.
    output = vr + ((vr == vm) || (lastRemovedDigit >= 5));
  }
  uint32_t olength = vplength - removed;

  // Step 5: Print the decimal representation.
  int index = 0;
  if (sign) {
    result[index++] = '-';
  }

#ifndef NO_DIGIT_TABLE
  // Print decimal digits after the decimal point.
  uint32_t i = 0;
  while (output >= 10000) {
    uint32_t c = output - 10000 * (output / 10000); // output % 10000;
    output /= 10000;
    uint32_t c0 = (c % 100) << 1;
    uint32_t c1 = (c / 100) << 1;
    memcpy(result + index + olength - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result + index + olength - i - 3, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (output >= 100) {
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
    result[index] = (char) ('0' + output);
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
  if (exp >= 10) {
    memcpy(result + index, DIGIT_TABLE + (2 * exp), 2);
    index += 2;
  } else {
    result[index++] = (char) ('0' + exp);
  }
#else
  if (exp >= 10) {
    result[index++] = '0' + (exp / 10) % 10;
  }
  result[index++] = '0' + exp % 10;
#endif // NO_DIGIT_TABLE

  // Terminate the string.
  result[index++] = '\0';
}

char* f2s(float f) {
  char* result = (char*) calloc(16, sizeof(char));
  f2s_buffered(f, result);
  return result;
}
