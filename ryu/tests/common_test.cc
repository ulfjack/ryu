// Copyright 2019 Ulf Adams
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

#include "ryu/common.h"
#include "third_party/gtest/gtest.h"

TEST(CommonTest, decimalLength9) {
  EXPECT_EQ(1u, decimalLength9(0));
  EXPECT_EQ(1u, decimalLength9(1));
  EXPECT_EQ(1u, decimalLength9(9));
  EXPECT_EQ(2u, decimalLength9(10));
  EXPECT_EQ(2u, decimalLength9(99));
  EXPECT_EQ(3u, decimalLength9(100));
  EXPECT_EQ(3u, decimalLength9(999));
  EXPECT_EQ(9u, decimalLength9(999999999));
}

TEST(CommonTest, ceil_log2pow5) {
  EXPECT_EQ(1, ceil_log2pow5(0));
  EXPECT_EQ(3, ceil_log2pow5(1));
  EXPECT_EQ(5, ceil_log2pow5(2));
  EXPECT_EQ(7, ceil_log2pow5(3));
  EXPECT_EQ(10, ceil_log2pow5(4));
  EXPECT_EQ(8192, ceil_log2pow5(3528));
}

TEST(CommonTest, log10Pow2) {
  EXPECT_EQ(0u, log10Pow2(0));
  EXPECT_EQ(0u, log10Pow2(1));
  EXPECT_EQ(0u, log10Pow2(2));
  EXPECT_EQ(0u, log10Pow2(3));
  EXPECT_EQ(1u, log10Pow2(4));
  EXPECT_EQ(496u, log10Pow2(1650));
}

TEST(CommonTest, log10Pow5) {
  EXPECT_EQ(0u, log10Pow5(0));
  EXPECT_EQ(0u, log10Pow5(1));
  EXPECT_EQ(1u, log10Pow5(2));
  EXPECT_EQ(2u, log10Pow5(3));
  EXPECT_EQ(2u, log10Pow5(4));
  EXPECT_EQ(1831u, log10Pow5(2620));
}

TEST(CommonTest, copy_special_str) {
  char buffer[100];
  memset(buffer, '\0', 100);
  EXPECT_EQ(3, copy_special_str(buffer, false, false, true));
  EXPECT_STREQ("NaN", buffer);

  memset(buffer, '\0', 100);
  EXPECT_EQ(8, copy_special_str(buffer, false, true, false));
  EXPECT_STREQ("Infinity", buffer);

  memset(buffer, '\0', 100);
  EXPECT_EQ(9, copy_special_str(buffer, true, true, false));
  EXPECT_STREQ("-Infinity", buffer);

  memset(buffer, '\0', 100);
  EXPECT_EQ(3, copy_special_str(buffer, false, false, false));
  EXPECT_STREQ("0E0", buffer);

  memset(buffer, '\0', 100);
  EXPECT_EQ(4, copy_special_str(buffer, true, false, false));
  EXPECT_STREQ("-0E0", buffer);
}

TEST(CommonTest, float_to_bits) {
  EXPECT_EQ(0u, float_to_bits(0.0f));
  EXPECT_EQ(0x40490fdau, float_to_bits(3.1415926f));
}

TEST(CommonTest, double_to_bits) {
  EXPECT_EQ(0ull, double_to_bits(0.0));
  EXPECT_EQ(0x400921FB54442D18ull, double_to_bits(3.1415926535897932384626433));
}


