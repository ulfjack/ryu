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

#include "ryu/ryu_parse.h"
#include "third_party/gtest/gtest.h"

TEST(S2dTest, Basic) {
  EXPECT_EQ(0.0, s2d("0"));
  EXPECT_EQ(-0.0, s2d("-0"));
  EXPECT_EQ(1.0, s2d("1"));
  EXPECT_EQ(2.0, s2d("2"));
  EXPECT_EQ(123456789.0, s2d("123456789"));
  EXPECT_EQ(123.456, s2d("123.456"));
  EXPECT_EQ(123.456, s2d("123456e-3"));
  EXPECT_EQ(123.456, s2d("1234.56e-1"));
  EXPECT_EQ(1.453, s2d("1.453"));
  EXPECT_EQ(1453.0, s2d("1.453e+3"));
}

TEST(S2dTest, MinMax) {
  EXPECT_EQ(1.7976931348623157e308, s2d("1.7976931348623157e308"));
  EXPECT_EQ(5E-324, s2d("5E-324"));
}

TEST(S2dTest, MantissaRoundingOverflow) {
  // This results in binary mantissa that is all ones and requires rounding up
  // because it is closer to 1 than to the next smaller float. This is a
  // regression test that the mantissa overflow is handled correctly by
  // increasing the exponent.
  EXPECT_EQ(1.0, s2d("0.99999999999999999"));
  // This number overflows the mantissa *and* the IEEE exponent.
  EXPECT_EQ(INFINITY, s2d("1.7976931348623159e308"));
}

TEST(S2dTest, Underflow) {
  EXPECT_EQ(0.0, s2d("2.4e-324"));
  EXPECT_EQ(0.0, s2d("1e-324"));
  EXPECT_EQ(0.0, s2d("9.99999e-325"));
}

TEST(S2dTest, Overflow) {
  EXPECT_EQ(INFINITY, s2d("2e308"));
  EXPECT_EQ(INFINITY, s2d("1e309"));
}

TEST(S2dTest, TableSizeDenormal) {
  EXPECT_EQ(5e-324, s2d("4.9406564584124654e-324"));
}
