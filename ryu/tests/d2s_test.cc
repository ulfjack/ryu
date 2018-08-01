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

#include "ryu/ryu.h"
#include "third_party/gtest/gtest.h"

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

TEST(D2sTest, Basic) {
  ASSERT_STREQ("0E0", d2s(0.0));
  ASSERT_STREQ("-0E0", d2s(-0.0));
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

TEST(D2sTest, LotsOfTrailingZeros) {
  ASSERT_STREQ("2.9802322387695312E-8", d2s(2.98023223876953125E-8));
}

TEST(D2sTest, Regression) {
  ASSERT_STREQ("-2.109808898695963E16", d2s(-2.109808898695963E16));
  ASSERT_STREQ("4.940656E-318", d2s(4.940656E-318));
  ASSERT_STREQ("1.18575755E-316", d2s(1.18575755E-316));
  ASSERT_STREQ("2.989102097996E-312", d2s(2.989102097996E-312));
  ASSERT_STREQ("9.0608011534336E15", d2s(9.0608011534336E15));
  ASSERT_STREQ("4.708356024711512E18", d2s(4.708356024711512E18));
  ASSERT_STREQ("9.409340012568248E18", d2s(9.409340012568248E18));
  ASSERT_STREQ("1.2345678E0", d2s(1.2345678));
}

TEST(D2sTest, OutputLength) {
  ASSERT_STREQ("1E0", d2s(1)); // already tested in Basic
  ASSERT_STREQ("1.2E0", d2s(1.2));
  ASSERT_STREQ("1.23E0", d2s(1.23));
  ASSERT_STREQ("1.234E0", d2s(1.234));
  ASSERT_STREQ("1.2345E0", d2s(1.2345));
  ASSERT_STREQ("1.23456E0", d2s(1.23456));
  ASSERT_STREQ("1.234567E0", d2s(1.234567));
  ASSERT_STREQ("1.2345678E0", d2s(1.2345678)); // already tested in Regression
  ASSERT_STREQ("1.23456789E0", d2s(1.23456789));
  ASSERT_STREQ("1.234567895E0", d2s(1.234567895)); // 1.234567890 would be trimmed
  ASSERT_STREQ("1.2345678901E0", d2s(1.2345678901));
  ASSERT_STREQ("1.23456789012E0", d2s(1.23456789012));
  ASSERT_STREQ("1.234567890123E0", d2s(1.234567890123));
  ASSERT_STREQ("1.2345678901234E0", d2s(1.2345678901234));
  ASSERT_STREQ("1.23456789012345E0", d2s(1.23456789012345));
  ASSERT_STREQ("1.234567890123456E0", d2s(1.234567890123456));
  ASSERT_STREQ("1.2345678901234567E0", d2s(1.2345678901234567));

  // Test 32-bit chunking
  ASSERT_STREQ("4.294967294E0", d2s(4.294967294)); // 2^32 - 2
  ASSERT_STREQ("4.294967295E0", d2s(4.294967295)); // 2^32 - 1
  ASSERT_STREQ("4.294967296E0", d2s(4.294967296)); // 2^32
  ASSERT_STREQ("4.294967297E0", d2s(4.294967297)); // 2^32 + 1
  ASSERT_STREQ("4.294967298E0", d2s(4.294967298)); // 2^32 + 2
}
