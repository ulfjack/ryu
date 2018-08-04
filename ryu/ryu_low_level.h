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
#ifndef RYU_LOW_LEVEL_H
#define RYU_LOW_LEVEL_H

// This is a low-level API for Ryu.
// It is still experimental and subject to change at any time.

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum float_type {
  INFINITY,
  NAN,
  VALUE
};

// A floating decimal representing (-1)^s * m * 10^e.
struct floating_decimal {
  uint64_t mantissa;
  int32_t exponent;
  bool sign;
  enum float_type type;
};

// A generic binary to decimal floating point to decimal conversion routine.
struct floating_decimal generic_binary_to_decimal(
    const uint64_t ieeeBits, const uint32_t mantissaBits, const uint32_t exponentBits);

#ifdef __cplusplus
}
#endif

#endif // RYU_LOW_LEVEL_H
