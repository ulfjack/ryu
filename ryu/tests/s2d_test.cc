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

#define EXPECT_S2D(a, b) { double value; EXPECT_EQ(SUCCESS, s2d(b, &value)); EXPECT_EQ(a, value); } while (0);

TEST(S2dTest, BadInput) {
  double value;
  EXPECT_EQ(MALFORMED_INPUT, s2d("x", &value));
  EXPECT_EQ(MALFORMED_INPUT, s2d("1..1", &value));
  EXPECT_EQ(MALFORMED_INPUT, s2d("..", &value));
  EXPECT_EQ(MALFORMED_INPUT, s2d("1..1", &value));
  EXPECT_EQ(MALFORMED_INPUT, s2d("1ee1", &value));
  EXPECT_EQ(MALFORMED_INPUT, s2d("1e.1", &value));
  EXPECT_EQ(INPUT_TOO_SHORT, s2d("", &value));
  EXPECT_EQ(INPUT_TOO_LONG, s2d("123456789012345678", &value));
  EXPECT_EQ(INPUT_TOO_LONG, s2d("1e12345", &value));
}

TEST(S2dTest, Basic) {
  EXPECT_S2D(0.0, "0");
  EXPECT_S2D(-0.0, "-0");
  EXPECT_S2D(1.0, "1");
  EXPECT_S2D(2.0, "2");
  EXPECT_S2D(123456789.0, "123456789");
  EXPECT_S2D(123.456, "123.456");
  EXPECT_S2D(123.456, "123456e-3");
  EXPECT_S2D(123.456, "1234.56e-1");
  EXPECT_S2D(1.453, "1.453");
  EXPECT_S2D(1453.0, "1.453e+3");
  EXPECT_S2D(0.0, ".0");
  EXPECT_S2D(1.0, "1e0");
  EXPECT_S2D(1.0, "1E0");
  EXPECT_S2D(1.0, "000001.000000");
  EXPECT_S2D(0.2316419, "0.2316419");
}

TEST(S2dTest, MinMax) {
  EXPECT_S2D(1.7976931348623157e308, "1.7976931348623157e308");
  EXPECT_S2D(5E-324, "5E-324");
}

TEST(S2dTest, MantissaRoundingOverflow) {
  // This results in binary mantissa that is all ones and requires rounding up
  // because it is closer to 1 than to the next smaller float. This is a
  // regression test that the mantissa overflow is handled correctly by
  // increasing the exponent.
  EXPECT_S2D(1.0, "0.99999999999999999");
  // This number overflows the mantissa *and* the IEEE exponent.
  EXPECT_S2D(INFINITY, "1.7976931348623159e308");
}

TEST(S2dTest, Underflow) {
  EXPECT_S2D(0.0, "2.4e-324");
  EXPECT_S2D(0.0, "1e-324");
  EXPECT_S2D(0.0, "9.99999e-325");
  // These are just about halfway between 0 and the smallest float.
  // The first is just below the halfway point, the second just above.
  EXPECT_S2D(0.0, "2.4703282292062327e-324");
  EXPECT_S2D(5e-324, "2.4703282292062328e-324");
}

TEST(S2dTest, Overflow) {
  EXPECT_S2D(INFINITY, "2e308");
  EXPECT_S2D(INFINITY, "1e309");
}

TEST(S2dTest, TableSizeDenormal) {
  EXPECT_S2D(5e-324, "4.9406564584124654e-324");
}

TEST(S2dTest, Issue157) {
  EXPECT_S2D(1.2999999999999999E+154, "1.2999999999999999E+154");
}

TEST(S2dTest, Issue173) {
	// Denormal boundary
	EXPECT_S2D(2.2250738585072012e-308, "2.2250738585072012e-308");
	EXPECT_S2D(2.2250738585072013e-308, "2.2250738585072013e-308");
	EXPECT_S2D(2.2250738585072014e-308, "2.2250738585072014e-308");
}