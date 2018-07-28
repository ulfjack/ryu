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
#ifndef RYU_TO_DECIMAL_H
#define RYU_TO_DECIMAL_H

#include "ryu/ryu.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_DIGIT_TABLE
#include "ryu/digit_table.h"
#endif

// Writes the decimal representation of the 32-bit unsigned integer, starting
// with the least significant digit, writing *right the left*.
// The char-pointer points at the rightmost, so first written, digit, and
// moves to the left.
// The leading digit is *not* written, but returned instead.
inline static char u32_to_decimal_rtl(uint64_t output, char * result) {
#ifndef NO_DIGIT_TABLE
  int i = 0;
  while (output >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
    const uint32_t c = output - 10000 * (output / 10000);
#else
    const uint32_t c = output % 10000;
#endif
    output /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    memcpy(result - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result - i - 3, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (output >= 100) {
    const uint32_t c = (output % 100) << 1;
    output /= 100;
    memcpy(result - i - 1, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (output >= 10) {
    const uint32_t c = output << 1;
    result[-i] = DIGIT_TABLE[c + 1];
    return DIGIT_TABLE[c];
  } else {
    // Print the leading decimal digit.
    return (char) ('0' + output);
  }
#else
  // Print decimal digits after the decimal point.
  while (output > 10) {
    const uint32_t c = output % 10; output /= 10;
    result[-i] = (char) ('0' + c);
  }
  // Print the leading decimal digit.
  return '0' + output % 10;
#endif // NO_DIGIT_TABLE
}

// Writes the decimal representation of the 64-bit unsigned integer, starting
// with the least significant digit, writing *right the left*.
// The char-pointer points at the rightmost, so first written, digit, and
// moves to the left.
// The leading digit is *not* written, but returned instead.
inline static char u64_to_decimal_rtl(uint64_t output, char * result) {
#ifndef NO_DIGIT_TABLE
  int i = 0;
#if defined(_M_IX86) || defined(_M_ARM)
  // 64-bit division is inefficient on 32-bit platforms.
  uint32_t output2;
  while ((output >> 32) != 0) {
    // Expensive 64-bit division.
    output2 = (uint32_t) (output % 1000000000);
    output /= 1000000000;

    // Cheap 32-bit divisions.
    const uint32_t c = output2 % 10000;
    output2 /= 10000;
    const uint32_t d = output2 % 10000;
    output2 /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    const uint32_t d0 = (d % 100) << 1;
    const uint32_t d1 = (d / 100) << 1;
    memcpy(result - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result - i - 3, DIGIT_TABLE + c1, 2);
    memcpy(result - i - 5, DIGIT_TABLE + d0, 2);
    memcpy(result - i - 7, DIGIT_TABLE + d1, 2);
    result[-i - 8] = (char) ('0' + output2);
    i += 9;
  }
  output2 = (uint32_t) output;
#else // ^^^ known 32-bit platforms ^^^ / vvv other platforms vvv
  // 64-bit division is efficient on 64-bit platforms.
  uint64_t output2 = output;
#endif // ^^^ other platforms ^^^
  while (output2 >= 10000) {
#ifdef __clang__ // https://bugs.llvm.org/show_bug.cgi?id=38217
    const uint32_t c = (uint32_t) (output2 - 10000 * (output2 / 10000));
#else
    const uint32_t c = (uint32_t) (output2 % 10000);
#endif
    output2 /= 10000;
    const uint32_t c0 = (c % 100) << 1;
    const uint32_t c1 = (c / 100) << 1;
    memcpy(result - i - 1, DIGIT_TABLE + c0, 2);
    memcpy(result - i - 3, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (output2 >= 100) {
    const uint32_t c = (uint32_t) ((output2 % 100) << 1);
    output2 /= 100;
    memcpy(result - i - 1, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (output2 >= 10) {
    const uint32_t c = (uint32_t) (output2 << 1);
    result[-i] = DIGIT_TABLE[c + 1];
    return DIGIT_TABLE[c];
  } else {
    // The leading decimal digit.
    return '0' + output2;
  }
#else
  // Print decimal digits after the decimal point.
  while (output > 10) {
    const uint32_t c = output % 10; output /= 10;
    *result-- = (char) ('0' + c);
  }
  // The leading decimal digit.
  return '0' + output % 10;
#endif // NO_DIGIT_TABLE
}

// Write a decimal representaion of a at most 2-digit integer to `result`,
// returning the number of characters written (1 or 2).
static inline int below100_to_decimal(uint8_t output, char * result) {
#ifndef NO_DIGIT_TABLE
  if (output >= 10) {
    memcpy(result, DIGIT_TABLE + (2 * output), 2);
    return 2;
  } else {
    result[0] = (char) ('0' + output);
    return 1;
  }
#else
  int index = 0;
  if (output >= 10) {
    result[index++] = '0' + (output / 10) % 10;
  }
  result[index++] = '0' + output % 10;
  return index;
#endif // NO_DIGIT_TABLE
}

// Write a decimal representaion of a at most 3-digit integer to `result`,
// returning the number of characters written (1, 2, or 3).
static inline int below1000_to_decimal(uint16_t output, char * result) {
#ifndef NO_DIGIT_TABLE
  if (output < 100) {
    return below100_to_decimal(output, result);
  } else {
    const int32_t c = output % 10;
    memcpy(result, DIGIT_TABLE + (2 * (output / 10)), 2);
    result[2] = (char) ('0' + c);
    return 3;
  }
#else
  if (output < 100) {
    return below100_to_decimal(output, result);
  } else {
    result[0] = (char) ('0' + output / 100);
    result[1] = '0' + (output / 10) % 10;
    result[2] = '0' + output % 10;
  }
#endif // NO_DIGIT_TABLE
}

#endif // RYU_TO_DECIMAL_H
