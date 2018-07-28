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

#include "ryu/common.h"
#include "ryu/ryu.h"
#include "ryu/to_decimal.h"

static inline int decimalLength(const uint64_t v) {
  // This is slightly faster than a loop.
  // The average output length is 16.38 digits, so we check high-to-low.
  // Function precondition: v is not an 18, 19, or 20-digit number.
  // (17 digits are sufficient for round-tripping.)
  assert(v < 100000000000000000L);
  if (v >= 10000000000000000L) { return 17; }
  if (v >= 1000000000000000L) { return 16; }
  if (v >= 100000000000000L) { return 15; }
  if (v >= 10000000000000L) { return 14; }
  if (v >= 1000000000000L) { return 13; }
  if (v >= 100000000000L) { return 12; }
  if (v >= 10000000000L) { return 11; }
  if (v >= 1000000000L) { return 10; }
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

int exp10d_to_string(struct exp10double f, char* result) {
  // Step 5: Print the decimal representation.

  if (f.exp10 == INT16_MAX || (f.value == 0 && f.exp10 == 0)) {
    return copy_special_str(result, f.sign, f.exp10, f.value);
  }

  int index = 0;
  if (f.sign) {
    result[index++] = '-';
  }

  int olength = decimalLength(f.value);
  char first_digit = u64_to_decimal_rtl(f.value, result + index + olength);
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
  index += below1000_to_decimal(f.exp10, result + index);

  return index;
}

int d2s_buffered_n(double f, char* result) {
  return exp10d_to_string(d2exp10(f), result);
}

void d2s_buffered(double f, char* result) {
  int index = d2s_buffered_n(f, result);

  // Terminate the string.
  result[index] = '\0';
}

char* d2s(double f) {
  char* const result = (char*) malloc(25);
  d2s_buffered(f, result);
  return result;
}
