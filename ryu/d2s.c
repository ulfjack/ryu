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

int dparts2s_buffered_n(struct dparts parts, char* result) {
  // Step 5: Print the decimal representation.

  if (parts.exp == INT16_MAX || (parts.output == 0 && parts.exp == 0)) {
    return copy_special_str(result, parts.sign, parts.exp, parts.output);
  }

  int index = 0;
  if (parts.sign) {
    result[index++] = '-';
  }

  int olength = decimalLength(parts.output);
  char first_digit = u64_to_decimal_rtl(parts.output, result + index + olength);
  result[index++] = first_digit;

  // Print decimal point if needed.
  if (olength > 1) {
    result[index] = '.';
    index += olength;
  }

  // Print the exponent.
  parts.exp += olength - 1;
  result[index++] = 'E';
  if (parts.exp < 0) {
    result[index++] = '-';
    parts.exp = -parts.exp;
  }
  index += below1000_to_decimal(parts.exp, result + index);

  return index;
}

int d2s_buffered_n(double f, char* result) {
  return dparts2s_buffered_n(d2parts(f), result);
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
