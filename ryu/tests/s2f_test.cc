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

#define EXPECT_S2F(a, b) do { float value; EXPECT_EQ(SUCCESS, s2f(b, &value)); EXPECT_EQ(a, value); } while (0);

TEST(S2fTest, Basic) {
  EXPECT_S2F(0.0f, "0");
  EXPECT_S2F(-0.0f, "-0");
  EXPECT_S2F(1.0f, "1");
  EXPECT_S2F(-1.0f, "-1");
  EXPECT_S2F(123456792.0f, "123456789");
  EXPECT_S2F(299792448.0f, "299792458");
}

TEST(S2fTest, MinMax) {
  EXPECT_S2F(1e-45f, "1e-45");
  EXPECT_S2F(FLT_MIN, "1.1754944e-38");
  EXPECT_S2F(FLT_MAX, "3.4028235e+38");
}

TEST(S2fTest, MantissaRoundingOverflow) {
  EXPECT_S2F(1.0f, "0.999999999");
  EXPECT_S2F(INFINITY, "3.4028236e+38");
  EXPECT_S2F(1.1754944e-38f, "1.17549430e-38"); // FLT_MIN
  EXPECT_S2F(1.1754944e-38f, "1.17549431e-38");
  EXPECT_S2F(1.1754944e-38f, "1.17549432e-38");
  EXPECT_S2F(1.1754944e-38f, "1.17549433e-38");
  EXPECT_S2F(1.1754944e-38f, "1.17549434e-38");
  EXPECT_S2F(1.1754944e-38f, "1.17549435e-38");
}

TEST(S2fTest, TrailingZeros) {
  EXPECT_S2F(26843550.0f, "26843549.5");
  EXPECT_S2F(50000004.0f, "50000002.5");
  EXPECT_S2F(99999992.0f, "99999989.5");
}
