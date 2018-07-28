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

#include <assert.h>

#include "ryu/ryu.h"
#include "ryu/common.h"
#include "ryu/to_decimal.h"

static inline uint32_t decimalLength(const uint32_t v) {
  // Function precondition: v is not a 10-digit number.
  // (9 digits are sufficient for round-tripping.)
  assert(v < 1000000000);
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

int exp10f_to_string(struct exp10float f, char* result) {
  // Step 5: Print the decimal representation.

  if (f.exp10 == INT16_MAX || (f.value == 0 && f.exp10 == 0)) {
    return copy_special_str(result, f.sign, f.exp10, f.value);
  }

  int index = 0;
  if (f.sign) {
    result[index++] = '-';
  }

  int olength = decimalLength(f.value);
  char first_digit = u32_to_decimal_rtl(f.value, result + index + olength);
  result[index++] = first_digit;

  // Print decimal point if needed.
  if (olength > 1) {
    result[index] = '.';
    index += olength;
  }

  // Print the exponent.
  f.exp10 += olength - 1;
  result[index++] = 'E';
  if (f.exp10 < 0) {
    result[index++] = '-';
    f.exp10 = -f.exp10;
  }
  index += below100_to_decimal(f.exp10, result + index);

  return index;
}

int f2s_buffered_n(float f, char* result) {
  return exp10f_to_string(f2exp10(f), result);
}

void f2s_buffered(float f, char* result) {
  int index = f2s_buffered_n(f, result);

  // Terminate the string.
  result[index] = '\0';
}

char* f2s(float f) {
  char* const result = (char*) malloc(16);
  f2s_buffered(f, result);
  return result;
}
