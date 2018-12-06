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

#include <math.h>

#include "ryu/ryu2.h"
#include "third_party/gtest/gtest.h"

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

static double ieeeParts2Double(const bool sign, const uint32_t ieeeExponent, const uint64_t ieeeMantissa) {
  assert(ieeeExponent <= 2047);
  assert(ieeeMantissa <= ((uint64_t)1 << 53) - 1);
  return int64Bits2Double(((uint64_t)sign << 63) | ((uint64_t)ieeeExponent << 52) | ieeeMantissa);
}

TEST(D2fixedTest, Basic) {
  ASSERT_STREQ(
    "3291009114715486435425664845573426149758869524108446525879746560",
    d2fixed(ieeeParts2Double(false, 1234, 99999), 0));
}
