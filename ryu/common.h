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
#include <string.h>

#if defined(_M_IX86) || defined(_M_ARM)
#define RYU_32_BIT_PLATFORM
#endif

// Returns the number of decimal digits in v, which must not contain more than 9 digits.
static inline uint32_t decimalLength9(const uint32_t v) {
  // Function precondition: v is not a 10-digit number.
  // (f2s: 9 digits are sufficient for round-tripping.)
  // (d2fixed: We print 9-digit blocks.)
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

// Returns e == 0 ? 1 : [log_2(5^e)]; requires 0 <= e <= 3528.
static inline int32_t log2pow5(const int32_t e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t) ((((uint32_t) e) * 1217359) >> 19);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int32_t pow5bits(const int32_t e) {
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t) (((((uint32_t) e) * 1217359) >> 19) + 1);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
static inline int32_t ceil_log2pow5(const int32_t e) {
  return log2pow5(e) + 1;
}

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
static inline uint32_t log10Pow2(const int32_t e) {
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  assert(e >= 0);
  assert(e <= 1650);
  return (((uint32_t) e) * 78913) >> 18;
}

// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
static inline uint32_t log10Pow5(const int32_t e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  assert(e >= 0);
  assert(e <= 2620);
  return (((uint32_t) e) * 732923) >> 20;
}


// a target string writer
typedef struct
{
  char * str;
  int size;
  int pos;
} tgt_str;

inline void tgt_init_empty(tgt_str *t)
{
  t->str = NULL;
  t->size = 0;
  t->pos = 0;
}

inline void tgt_init(tgt_str *t, char *tgt, int size)
{
  t->str = tgt;
  t->size = size;
  t->pos = 0;
}

inline void tgt_write_char(tgt_str *t, int dst_pos, char c)
{
  if(dst_pos < t->size)
  {
    t->str[dst_pos] = c;
  }
}

inline void tgt_append_char(tgt_str *t, char c)
{
  if(t->pos < t->size)
  {
    t->str[t->pos] = c;
  }
  ++t->pos;
}

inline void tgt_append_char_n(tgt_str *t, char c, int num_chars)
{
  assert(num_chars >= 0);
  int remainder = t->size >= t->pos ? t->size - t->pos : 0;
  int num = num_chars <= remainder ? num_chars : remainder;
  for(int i = 0; i < num; ++i)
  {
      t->str[t->pos + i] = c;
  }
  t->pos += num_chars;
}

inline void tgt_append_str(tgt_str *t, const char *str, int size)
{
  assert(size >= 0);
  int remainder = t->size >= t->pos ? t->size - t->pos : 0;
  int num = size <= remainder ? size : remainder;
  memcpy(t->str + t->pos, str, num);
  t->pos += size;
}

inline void tgt_write_str(tgt_str *t, int dst_pos, const char *str, int size)
{
  assert(size >= 0);
  int remainder = t->size >= t->pos ? t->size - t->pos : 0;
  int num = size <= remainder ? size : remainder;
  memcpy(t->str + dst_pos, str, num);
}


static inline void tgt_copy_special_str(tgt_str *result, const bool sign, const bool exponent, const bool mantissa) {
  if (mantissa) {
    tgt_append_str(result, "NaN", 3);
    return;
  }
  if (sign) {
    tgt_append_char(result, '-');
  }
  if (exponent) {
    tgt_append_str(result, "Infinity", 8);
    return;
  }
  tgt_append_str(result, "0E0", 3);
  return;
}


static inline int copy_special_str(char * const result, const bool sign, const bool exponent, const bool mantissa) {
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

static inline uint32_t float_to_bits(const float f) {
  uint32_t bits = 0;
  memcpy(&bits, &f, sizeof(float));
  return bits;
}

static inline uint64_t double_to_bits(const double d) {
  uint64_t bits = 0;
  memcpy(&bits, &d, sizeof(double));
  return bits;
}

#endif // RYU_COMMON_H
