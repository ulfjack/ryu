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
#include "third_party/double-conversion/double-conversion/double-conversion.h"

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

static bool compareRyuWithDC(double f) {
  using double_conversion::DoubleToStringConverter;
  using double_conversion::StringBuilder;

  static DoubleToStringConverter converter(
      DoubleToStringConverter::Flags::EMIT_TRAILING_DECIMAL_POINT
          | DoubleToStringConverter::Flags::EMIT_TRAILING_ZERO_AFTER_POINT,
      "Infinity", "NaN", 'E', 7, 7, 0, 0);

  char bufDC[32];
  StringBuilder builder(bufDC, 32);
  converter.ToShortest(f, &builder);
  builder.Finalize();

  char bufRyu[32];
  d2s_buffered(f, bufRyu);

  return strcmp(bufDC, bufRyu) == 0;
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

TEST(D2sTest, LooksLikePow5) {
  // These numbers have a mantissa that is a multiple of the largest power of 5 that fits,
  // and an exponent that causes the computation for q to result in 22, which is a corner
  // case for Ryu.
  ASSERT_STREQ("5.764607523034235E39", d2s(int64Bits2Double(0x4830F0CF064DD592)));
  ASSERT_STREQ("1.152921504606847E40", d2s(int64Bits2Double(0x4840F0CF064DD592)));
  ASSERT_STREQ("2.305843009213694E40", d2s(int64Bits2Double(0x4850F0CF064DD592)));
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

TEST(D2sTest, Boundaries) {
  const uint64_t eMin = 1;
  const uint64_t eMax = 2047; // Tests NaN and Infinity, too.
  for (uint64_t e = eMin; e <= eMax; ++e) {
    ASSERT_TRUE(compareRyuWithDC(int64Bits2Double((e-1) << 52 | 0x000FFFFFFFFFFFFF)));
    ASSERT_TRUE(compareRyuWithDC(int64Bits2Double((e  ) << 52 | 0x0000000000000000)));
    ASSERT_TRUE(compareRyuWithDC(int64Bits2Double((e  ) << 52 | 0x0000000000000001)));
  }
}
