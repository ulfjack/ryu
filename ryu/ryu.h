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
#ifndef RYU_H
#define RYU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Represents (-1)^sign * value * 10^exp10.
struct exp10double {
  bool sign;
  int16_t exp10;
  uint64_t value;
};
struct exp10float {
  bool sign;
  int16_t exp10;
  uint32_t value;
};

// Convert a float/double to a representation with a 10-based exponent.
struct exp10double d2exp10(double f);
struct exp10float f2exp10(float f);

// Convert the result of the above function to a decimal representation.
// The result is written to `result`, and the number of written characters is returned.
// The result is not zero-terminated.
int exp10d_to_string(struct exp10double value, char* result); // Writes max 24 chars.
int exp10f_to_string(struct exp10float value, char* result); // Writes max 15 chars.

// Does both steps at once.
int d2s_buffered_n(double f, char* result);
int f2s_buffered_n(float f, char* result);

// Zero-terminates the output.
void d2s_buffered(double f, char* result);
void f2s_buffered(float f, char* result);

// Allocates a new buffer with malloc(). free() it yourself.
char* d2s(double f);
char* f2s(float f);

#ifdef __cplusplus
}
#endif

#endif // RYU_H
