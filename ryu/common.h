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
#ifndef RYU_COMMON_H
#define RYU_COMMON_H

#include <assert.h>
#include <stdint.h>

// Returns e == 0 ? 1 : ceil(log_2(5^e)).
static inline uint32_t pow5bits(const int32_t e) {
  // This function has only been tested for 0 <= e <= 1500.
  assert(e >= 0);
  assert(e <= 1500);
  return ((((uint32_t) e) * 1217359) >> 19) + 1;
}

// Returns floor(log_10(2^e)).
static inline int32_t log10Pow2(const int32_t e) {
  // This function has only been tested for 0 <= e <= 1500.
  assert(e >= 0);
  assert(e <= 1500);
  return (int32_t) ((((uint32_t) e) * 78913) >> 18);
}

// Returns floor(log_10(5^e)).
static inline int32_t log10Pow5(const int32_t e) {
  // This function has only been tested for 0 <= e <= 1500.
  assert(e >= 0);
  assert(e <= 1500);
  return (int32_t) ((((uint32_t) e) * 732923) >> 20);
}

static inline int copy_special_str(char * result, bool sign, bool exponent, bool mantissa) {
  if (mantissa) {
    memcpy(result, "NaN", 3);
    return 3;
  }
  if (sign) {
    result[0] = '-';
  }
  if (exponent) {
    memcpy(result + sign, "Infinity", 8);
    return sign + 8;
  }
  memcpy(result + sign, "0E0", 3);
  return sign + 3;
}

#endif // RYU_COMMON_H
