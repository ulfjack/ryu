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
#ifndef RYU_D2S_H
#define RYU_D2S_H

#include <inttypes.h>

// Only include the full table if we're not optimizing for size.
#if !defined(RYU_OPTIMIZE_SIZE)
#include "ryu/d2s_full_table.h"
#endif

#if defined(HAS_UINT128)
typedef __uint128_t uint128_t;
#else
#include "ryu/mulshift128.h"
#endif

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11

// These have to be 64-bit constants in order for the computations to be safe
// given the ranges they have to handle for 64-bit floating point numbers.
#define DOUBLE_LOG10_2_DENOMINATOR 10000000ull
#define DOUBLE_LOG10_2_NUMERATOR 3010299ull // DOUBLE_LOG10_2_DENOMINATOR * log_10(2)
#define DOUBLE_LOG10_5_DENOMINATOR 10000000ull
#define DOUBLE_LOG10_5_NUMERATOR 6989700ull // DOUBLE_LOG10_5_DENOMINATOR * log_10(5)
#define DOUBLE_LOG2_5_DENOMINATOR 10000000ull
#define DOUBLE_LOG2_5_NUMERATOR 23219280ull // DOUBLE_LOG2_5_DENOMINATOR * log_2(5)

static inline uint32_t double_pow5bits(int32_t e) {
  return e == 0
      ? 1
      // We need to round up in this case.
      : (uint32_t) ((e * DOUBLE_LOG2_5_NUMERATOR + DOUBLE_LOG2_5_DENOMINATOR - 1) / DOUBLE_LOG2_5_DENOMINATOR);
}

#define DOUBLE_POW5_INV_BITCOUNT 122
#define DOUBLE_POW5_BITCOUNT 121

#if defined(RYU_OPTIMIZE_SIZE)

#define POW5_TABLE_SIZE 26
static const uint64_t DOUBLE_POW5_TABLE[POW5_TABLE_SIZE] = {
1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull,
1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull, 6103515625ull,
30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull,
19073486328125ull, 95367431640625ull, 476837158203125ull,
2384185791015625ull, 11920928955078125ull, 59604644775390625ull,
298023223876953125ull //, 1490116119384765625ull
};

static const uint64_t DOUBLE_POW5_SPLIT2[13][2] = {
 {                    0u,    72057594037927936u },
 { 10376293541461622784u,    93132257461547851u },
 { 15052517733678820785u,   120370621524202240u },
 {  6258995034005762182u,    77787690973264271u },
 { 14893927168346708332u,   100538234169297439u },
 {  4272820386026678563u,   129942622070561240u },
 {  7330497575943398595u,    83973451344588609u },
 { 18377130505971182927u,   108533142064701048u },
 { 10038208235822497557u,   140275798336537794u },
 {  7017903361312433648u,    90651109995611182u },
 {  6366496589810271835u,   117163813585596168u },
 {  9264989777501460624u,    75715339914673581u },
 { 17074144231291089770u,    97859783203563123u },
};
// Unfortunately, the results are sometimes off by one. We use an additional
// lookup table to store those cases and adjust the result.
static const uint32_t POW5_OFFSETS[13] = {
0x00000000, 0x00000000, 0x00000000, 0x033c55be, 0x03db77d8, 0x0265ffb2,
0x00000800, 0x01a8ff56, 0x00000000, 0x0037a200, 0x00004000, 0x03fffffc,
0x00003ffe,
};


static const uint64_t DOUBLE_POW5_INV_SPLIT2[13][2] = {
 {                    1u,   288230376151711744u },
 {  7661987648932456967u,   223007451985306231u },
 { 12652048002903177473u,   172543658669764094u },
 {  5522544058086115566u,   266998379490113760u },
 {  3181575136763469022u,   206579990246952687u },
 {  4551508647133041040u,   159833525776178802u },
 {  1116074521063664381u,   247330401473104534u },
 { 17400360011128145022u,   191362629322552438u },
 {  9297997190148906106u,   148059663038321393u },
 { 11720143854957885429u,   229111231347799689u },
 { 15401709288678291155u,   177266229209635622u },
 {  3003071137298187333u,   274306203439684434u },
 { 17516772882021341108u,   212234145163966538u },
};
static const uint32_t POW5_INV_OFFSETS[20] = {
0x51505404, 0x55054514, 0x45555545, 0x05511411, 0x00505010, 0x00000004,
0x00000000, 0x00000000, 0x55555040, 0x00505051, 0x00050040, 0x55554000,
0x51659559, 0x00001000, 0x15000010, 0x55455555, 0x41404051, 0x00001010,
0x00000014, 0x00000000,
};

#if defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computePow5(uint32_t i, uint64_t* result) {
  uint32_t base = i / POW5_TABLE_SIZE;
  uint32_t base2 = base * POW5_TABLE_SIZE;
  uint32_t offset = i - base2;
  const uint64_t* mul = DOUBLE_POW5_SPLIT2[base];
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  uint64_t m = DOUBLE_POW5_TABLE[offset];
  uint128_t b0 = ((uint128_t) m) * mul[0];
  uint128_t b2 = ((uint128_t) m) * mul[1];
  uint32_t delta = double_pow5bits(i) - double_pow5bits(base2);
  uint128_t shiftedSum = (b0 >> delta) + (b2 << (64 - delta)) + ((POW5_OFFSETS[base] >> offset) & 1);
  result[0] = (uint64_t) shiftedSum;
  result[1] = (uint64_t) (shiftedSum >> 64);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computeInvPow5(uint32_t i, uint64_t* result) {
  uint32_t base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
  uint32_t base2 = base * POW5_TABLE_SIZE;
  uint32_t offset = base2 - i;
  const uint64_t* mul = DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  uint64_t m = DOUBLE_POW5_TABLE[offset]; // 5^offset
  uint128_t b0 = ((uint128_t) m) * (mul[0] - 1);
  uint128_t b2 = ((uint128_t) m) * mul[1]; // 1/5^base2 * 5^offset = 1/5^(base2-offset) = 1/5^i
  uint32_t delta = double_pow5bits(base2) - double_pow5bits(i);
  uint128_t shiftedSum =
    ((b0 >> delta) + (b2 << (64 - delta))) + 1 + ((POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[0] = (uint64_t) shiftedSum;
  result[1] = (uint64_t) (shiftedSum >> 64);
}

#else // defined(HAS_UINT128)

// Computes 5^i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computePow5(uint32_t i, uint64_t* result) {
  uint32_t base = i / POW5_TABLE_SIZE;
  uint32_t base2 = base * POW5_TABLE_SIZE;
  uint32_t offset = i - base2;
  const uint64_t* mul = DOUBLE_POW5_SPLIT2[base];
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  uint64_t m = DOUBLE_POW5_TABLE[offset];
  uint64_t high1;
  uint64_t low1 = umul128(m, mul[1], &high1);
  uint64_t high0;
  uint64_t low0 = umul128(m, mul[0], &high0);
  uint64_t sum = high0 + low1;
  if (sum < high0) high1++; // overflow into high1
  // high1 | sum | low0
  uint32_t delta = double_pow5bits(i) - double_pow5bits(base2);
  result[0] = shiftright128(low0, sum, delta) + ((POW5_OFFSETS[base] >> offset) & 1);
  result[1] = shiftright128(sum, high1, delta);
}

// Computes 5^-i in the form required by Ryu, and stores it in the given pointer.
static inline void double_computeInvPow5(uint32_t i, uint64_t* result) {
  uint32_t base = (i + POW5_TABLE_SIZE - 1) / POW5_TABLE_SIZE;
  uint32_t base2 = base * POW5_TABLE_SIZE;
  uint32_t offset = base2 - i;
  const uint64_t* mul = DOUBLE_POW5_INV_SPLIT2[base]; // 1/5^base2
  if (offset == 0) {
    result[0] = mul[0];
    result[1] = mul[1];
    return;
  }
  uint64_t m = DOUBLE_POW5_TABLE[offset];
  uint64_t high1;
  uint64_t low1 = umul128(m, mul[1], &high1);
  uint64_t high0;
  uint64_t low0 = umul128(m, mul[0] - 1, &high0);
  uint64_t sum = high0 + low1;
  if (sum < high0) high1++; // overflow into high1
  // high1 | sum | low0
  uint32_t delta = double_pow5bits(base2) - double_pow5bits(i);
  result[0] = shiftright128(low0, sum, delta) + 1 + ((POW5_INV_OFFSETS[i / 16] >> ((i % 16) << 1)) & 3);
  result[1] = shiftright128(sum, high1, delta);
}

#endif // defined(HAS_UINT128)

#endif // defined(RYU_OPTIMIZE_SIZE)

#endif // RYU_D2S_H
