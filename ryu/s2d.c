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
  return 63 - __builtin_clzl(value);
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
  int digitsAfterDot = 0;
  uint64_t m10 = 0;
  uint32_t e10 = 0;
  bool signedM = false;
  bool signedE = false;
  int i = 0;
  if (buffer[i] == '-') {
    signedM = true;
    i++;
  }
  for (; i < len; i++) {
    char c = buffer[i];
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
  if (i < len && buffer[i] == '.') {
    // TODO: Handle this case!
    return 0;
  }
  if (i < len && ((buffer[i] == 'e') || (buffer[i] == 'E'))) {
    // TODO: Handle this case!
    return 0;
  }
  if (i < len) {
    // TODO: Error handling!
    return 0;
  }
  e10 -= digitsAfterDot;
  if (m10 == 0) {
    return signedM ? -0.0 : 0.0;
  }

#ifdef RYU_DEBUG
  printf("Input=%s\n", buffer);
  printf("m10 * 10^e10 = %" PRIu64 " * 10^%d\n", m10, e10);
#endif

  int32_t e2;
  uint64_t m2;
  // Convert to binary float
  if (e10 >= 0) {
    // The length of m * 10^e in bits is
    //   log2(m10 * 10^e10) = log2(m10) + e10 log2(10) = log2(m10) + e10 + e10 * log2(5)
    // We want to compute the DOUBLE_MANTISSA_BITS + 1 top-most bits (+1 for the implicit leading
    // one in IEEE format). We therefore choose a binary output exponent of
    //   log2(m * 10^e) - (DOUBLE_MANTISSA_BITS + 1).
    // We use the floor of the binary logarithm so that we get at least this many bits; better to
    // have an additional bit than to not have enough bits.
    e2 = floor_log2(m10) + e10 + log2pow5(e10) - (DOUBLE_MANTISSA_BITS + 1);
    int j = e2 - e10 - ceil_log2pow5(e10) + DOUBLE_POW5_BITCOUNT;
    m2 = mulShift(m10, DOUBLE_POW5_SPLIT[e10], j);
  } else {
    // TODO: Implement me!
    return 0;
  }

#ifdef RYU_DEBUG
  printf("m2 * 2^e2 = %" PRIu64 " * 2^%d\n", m2, e2);
#endif

  uint32_t ieee_e2 = (uint32_t) max(0, e2 + DOUBLE_EXPONENT_BIAS + floor_log2(m2));
  int32_t shift = (ieee_e2 == 0 ? 1 : ieee_e2) - e2 - DOUBLE_EXPONENT_BIAS - DOUBLE_MANTISSA_BITS;
#ifdef RYU_DEBUG
  printf("ieee_e2 = %d\n", ieee_e2);
  printf("shift = %d\n", shift);
#endif
  
  uint64_t lastRemovedBit = ((m2 >> (shift - 1)) & 1);
  uint64_t ieee_m2 = (m2 >> shift) & ((1L << DOUBLE_MANTISSA_BITS) - 1);
  uint64_t ieee = (((((uint64_t) signedM) << DOUBLE_EXPONENT_BITS) | (uint64_t)ieee_e2) << DOUBLE_MANTISSA_BITS) | ieee_m2;
  return int64Bits2Double(ieee);
}

double s2d(const char * buffer) {
  return s2d_n(buffer, strlen(buffer));
}
