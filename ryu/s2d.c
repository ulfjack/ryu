// Copyright 2019 Ulf Adams
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

#include "ryu/ryu_parse.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif

#include "ryu/common.h"
#include "ryu/d2s_intrinsics.h"

#include "ryu/d2s_full_table.h"

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_EXPONENT_BIAS 1023

static inline uint32_t floor_log2(const uint64_t value) {
  return 63 - __builtin_clzll(value);
}

static inline int32_t max(int32_t a, int32_t b) {
  return a < b ? b : a;
}

static inline double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

double s2d_n(const char * buffer, const int len) {
  if (len == 0) {
    // TODO: Error handling?
    return 0;
  }
  int m10digits = 0;
  int e10digits = 0;
  int dotIndex = len;
  int eIndex = len;
  uint64_t m10 = 0;
  int32_t e10 = 0;
  bool signedM = false;
  bool signedE = false;
  int i = 0;
  if (buffer[i] == '-') {
    signedM = true;
    i++;
  }
  for (; i < len; i++) {
    char c = buffer[i];
    if (c == '.') {
      if (dotIndex != len) {
        // TODO: Error handling!
        return 0;
      }
      dotIndex = i;
      continue;
    }
    if ((c < '0') || (c > '9')) {
      break;
    }
    if (m10digits >= 17) {
      // TODO: Error handling!
      return 0;
    }
    m10 = 10 * m10 + (c - '0');
    if (m10 != 0) {
      m10digits++;
    }
  }
  if (i < len && ((buffer[i] == 'e') || (buffer[i] == 'E'))) {
    eIndex = i;
    i++;
    if (i < len && ((buffer[i] == '-') || (buffer[i] == '+'))) {
      signedE = buffer[i] == '-';
      i++;
    }
    for (; i < len; i++) {
      char c = buffer[i];
      if ((c < '0') || (c > '9')) {
        // TODO: Error handling!
        return 0;
      }
      if (e10digits > 3) {
        // TODO: Error handling!
        return 0;
      }
      e10 = 10 * e10 + (c - '0');
      if (e10 != 0) {
        e10digits++;
      }
    }
  }
  if (i < len) {
    // TODO: Error handling!
    return 0;
  }
  if (signedE) {
    e10 = -e10;
  }
  e10 -= dotIndex < eIndex ? eIndex - dotIndex - 1 : 0;
  if (m10 == 0) {
    return signedM ? -0.0 : 0.0;
  }

#ifdef RYU_DEBUG
  printf("Input=%s\n", buffer);
  printf("m10 * 10^e10 = %" PRIu64 " * 10^%d\n", m10, e10);
#endif

  // Convert to binary float m2 * 2^e2, while retaining information about whether the conversion
  // was exact (trailingZeros).
  int32_t e2;
  uint64_t m2;
  bool trailingZeros;
  if (e10 >= 0) {
    // The length of m * 10^e in bits is:
    //   log2(m10 * 10^e10) = log2(m10) + e10 log2(10) = log2(m10) + e10 + e10 * log2(5)
    //
    // We want to compute the DOUBLE_MANTISSA_BITS + 1 top-most bits (+1 for the implicit leading
    // one in IEEE format). We therefore choose a binary output exponent of
    //   log2(m10 * 10^e10) - (DOUBLE_MANTISSA_BITS + 1).
    //
    // We use floor(log2(5^e10)) so that we get at least this many bits; better to
    // have an additional bit than to not have enough bits.
    e2 = floor_log2(m10) + e10 + log2pow5(e10) - (DOUBLE_MANTISSA_BITS + 1);

    // We now compute [m10 * 10^e10 / 2^e2] = [m10 * 5^e10 / 2^(e2-e10)].
    // To that end, we use the DOUBLE_POW5_SPLIT table.
    int j = e2 - e10 - ceil_log2pow5(e10) + DOUBLE_POW5_BITCOUNT;
    assert(j >= 0);
    m2 = mulShift(m10, DOUBLE_POW5_SPLIT[e10], j);

    // We also compute if the result is exact, i.e.,
    //   [m10 * 10^e10 / 2^e2] == m10 * 10^e10 / 2^e2.
    // This can only be the case if 2^e2 divides m10 * 10^e10, which in turn requires that the
    // largest power of 2 that divides m10 + e10 is greater than e2. If e2 is less than e10, then
    // the result must be exact. Otherwise we use the existing multipleOfPowerOf2 function.
    trailingZeros = e2 < e10 || multipleOfPowerOf2(m10, e2 - e10);
  } else {
    e2 = floor_log2(m10) + e10 - ceil_log2pow5(-e10) - (DOUBLE_MANTISSA_BITS + 1);
    int j = e2 - e10 + ceil_log2pow5(-e10) - 1 + DOUBLE_POW5_INV_BITCOUNT;
    m2 = mulShift(m10, DOUBLE_POW5_INV_SPLIT[-e10], j);
    trailingZeros = multipleOfPowerOf5(m10, -e10);
  }

#ifdef RYU_DEBUG
  printf("m2 * 2^e2 = %" PRIu64 " * 2^%d\n", m2, e2);
#endif

  // Compute the final IEEE exponent.
  uint32_t ieee_e2 = (uint32_t) max(0, e2 + DOUBLE_EXPONENT_BIAS + floor_log2(m2));

  // We need to figure out how much we need to shift m2. The tricky part is that we need to take
  // the final IEEE exponent into account, so we need to reverse the bias and also special-case
  // the value 0.
  int32_t shift = (ieee_e2 == 0 ? 1 : ieee_e2) - e2 - DOUBLE_EXPONENT_BIAS - DOUBLE_MANTISSA_BITS;
  assert(shift >= 0);
#ifdef RYU_DEBUG
  printf("ieee_e2 = %d\n", ieee_e2);
  printf("shift = %d\n", shift);
#endif
  
  // We need to round up if the exact value is more than 0.5 above the value we computed. That's
  // equivalent to checking if the last removed bit was 1 and either the value was not just
  // trailing zeros or the result would otherwise be odd.
  //
  // We need to update trailingZeros given that we have the exact output exponent ieee_e2 now.
  trailingZeros &= (m2 & ((1ull << (shift - 1)) - 1)) == 0;
  uint64_t lastRemovedBit = (m2 >> (shift - 1)) & 1;
  bool roundUp = (lastRemovedBit != 0) && (!trailingZeros || (((m2 >> shift) & 1) != 0));

  uint64_t ieee_m2 = (m2 >> shift) + roundUp;
  if (ieee_m2 == (1ull << (DOUBLE_MANTISSA_BITS + 1))) {
    // Due to how the IEEE represents +/-Infinity, we don't need to check for overflow here.
    ieee_e2++;
  }
  ieee_m2 &= (1ull << DOUBLE_MANTISSA_BITS) - 1;
  uint64_t ieee = (((((uint64_t) signedM) << DOUBLE_EXPONENT_BITS) | (uint64_t)ieee_e2) << DOUBLE_MANTISSA_BITS) | ieee_m2;
  return int64Bits2Double(ieee);
}

double s2d(const char * buffer) {
  return s2d_n(buffer, strlen(buffer));
}
