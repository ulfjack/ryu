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
#ifndef RYU_MULSHIFT128_H
#define RYU_MULSHIFT128_H

#include <stdint.h>

#if defined(HAS_64_BIT_INTRINSICS)

#include <intrin.h>

static inline uint64_t umul128(const uint64_t a, const uint64_t b, uint64_t* const productHi) {
  return _umul128(a, b, productHi);
}

static inline uint64_t shiftright128(const uint64_t lo, const uint64_t hi, const uint32_t dist) {
  return __shiftright128(lo, hi, (unsigned char) dist);
}

#else // defined(HAS_64_BIT_INTRINSICS)

static inline uint64_t umul128(const uint64_t a, const uint64_t b, uint64_t* const productHi) {
  const uint64_t aLo = a & 0xffffffffu;
  const uint64_t aHi = a >> 32;
  const uint64_t bLo = b & 0xffffffffu;
  const uint64_t bHi = b >> 32;

  const uint64_t b00 = aLo * bLo;
  const uint64_t b01 = aLo * bHi;
  const uint64_t b10 = aHi * bLo;
  const uint64_t b11 = aHi * bHi;

  const uint64_t midSum = b01 + b10;
  const uint64_t midCarry = midSum < b01;

  const uint64_t productLo = b00 + (midSum << 32);
  const uint64_t productLoCarry = productLo < b00;

  *productHi = b11 + (midSum >> 32) + (midCarry << 32) + productLoCarry;
  return productLo;
}

static inline uint64_t shiftright128(const uint64_t lo, const uint64_t hi, const uint32_t dist) {
  // shift hi-lo right by 0 < dist < 128
  return (dist >= 64)
      ? hi >> (dist - 64)
      : (hi << (64 - dist)) | (lo >> dist);
}

#endif // defined(HAS_64_BIT_INTRINSICS)

#endif // RYU_MULSHIFT128_H
