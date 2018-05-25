// Copyright 2018 Ulf Adams
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <math.h>

#include "ryu/ryu.h"
#include "third_party/gtest/gtest.h"

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

TEST(D2sTest, Basic) {
  ASSERT_STREQ("0.0", d2s(0.0));
  #ifndef _WIN32
  ASSERT_STREQ("-0.0", d2s(-0.0));
  #else
  ASSERT_STREQ("-0.0", d2s(int64Bits2Double(0x8000000000000000)));
  #endif
  ASSERT_STREQ("1E0", d2s(1.0));
  ASSERT_STREQ("-1E0", d2s(-1.0));
  ASSERT_STREQ("NaN", d2s(NAN));
  ASSERT_STREQ("Infinity", d2s(INFINITY));
  ASSERT_STREQ("-Infinity", d2s(-INFINITY));
}

TEST(D2sTest, SwitchToSubnormal) {
  ASSERT_STREQ("2.2250738585072014E-308", d2s(2.2250738585072014E-308));
}

TEST(D2sTest, MinAndMax) {
  ASSERT_STREQ("1.7976931348623157E308", d2s(int64Bits2Double(0x7fefffffffffffff)));
  ASSERT_STREQ("5E-324", d2s(int64Bits2Double(1)));
}

TEST(D2sTest, Regression) {
  ASSERT_STREQ("-2.109808898695963E16", d2s(-2.109808898695963E16));
  ASSERT_STREQ("4.940656E-318", d2s(4.940656E-318));
  ASSERT_STREQ("1.18575755E-316", d2s(1.18575755E-316));
  ASSERT_STREQ("2.989102097996E-312", d2s(2.989102097996E-312));
  ASSERT_STREQ("9.0608011534336E15", d2s(9.0608011534336E15));
  ASSERT_STREQ("4.708356024711512E18", d2s(4.708356024711512E18));
  ASSERT_STREQ("9.409340012568248E18", d2s(9.409340012568248E18));
}
