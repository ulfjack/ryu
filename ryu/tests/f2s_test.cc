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

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

TEST(F2sTest, Basic) {
  ASSERT_STREQ("0E0", f2s(0.0));
  ASSERT_STREQ("-0E0", f2s(-0.0));
  ASSERT_STREQ("1E0", f2s(1.0));
  ASSERT_STREQ("-1E0", f2s(-1.0));
  ASSERT_STREQ("NaN", f2s(NAN));
  ASSERT_STREQ("Infinity", f2s(INFINITY));
  ASSERT_STREQ("-Infinity", f2s(-INFINITY));
}

TEST(F2sTest, SwitchToSubnormal) {
  ASSERT_STREQ("1.1754944E-38", f2s(1.1754944E-38f));
}

TEST(F2sTest, MinAndMax) {
  ASSERT_STREQ("3.4028235E38", f2s(int32Bits2Float(0x7f7fffff)));
  ASSERT_STREQ("1E-45", f2s(int32Bits2Float(1)));
}

// Check that we return the exact boundary if it is the shortest
// representation, but only if the original floating point number is even.
TEST(F2sTest, BoundaryRoundEven) {
  ASSERT_STREQ("3.355445E7", f2s(3.355445E7f));
  ASSERT_STREQ("9E9", f2s(8.999999E9f));
  ASSERT_STREQ("3.436672E10", f2s(3.4366717E10f));
}

// If the exact value is exactly halfway between two shortest representations,
// then we round to even. It seems like this only makes a difference if the
// last two digits are ...2|5 or ...7|5, and we cut off the 5.
TEST(F2sTest, ExactValueRoundEven) {
  ASSERT_STREQ("3.0540412E5", f2s(3.0540412E5f));
  ASSERT_STREQ("8.0990312E3", f2s(8.0990312E3f));
}

TEST(F2sTest, LotsOfTrailingZeros) {
  // Pattern for the first test: 00111001100000000000000000000000
  ASSERT_STREQ("2.4414062E-4", f2s(2.4414062E-4f));
  ASSERT_STREQ("2.4414062E-3", f2s(2.4414062E-3f));
  ASSERT_STREQ("4.3945312E-3", f2s(4.3945312E-3f));
  ASSERT_STREQ("6.3476562E-3", f2s(6.3476562E-3f));
}

TEST(F2sTest, Regression) {
  ASSERT_STREQ("4.7223665E21", f2s(4.7223665E21f));
  ASSERT_STREQ("8.388608E6", f2s(8388608.0f));
  ASSERT_STREQ("1.6777216E7", f2s(1.6777216E7f));
  ASSERT_STREQ("3.3554436E7", f2s(3.3554436E7f));
  ASSERT_STREQ("6.7131496E7", f2s(6.7131496E7f));
  ASSERT_STREQ("1.9310392E-38", f2s(1.9310392E-38f));
  ASSERT_STREQ("-2.47E-43", f2s(-2.47E-43f));
  ASSERT_STREQ("1.993244E-38", f2s(1.993244E-38f));
  ASSERT_STREQ("4.1039004E3", f2s(4103.9003f));
  ASSERT_STREQ("5.3399997E9", f2s(5.3399997E9f));
  ASSERT_STREQ("6.0898E-39", f2s(6.0898E-39f));
  ASSERT_STREQ("1.0310042E-3", f2s(0.0010310042f));
  ASSERT_STREQ("2.882326E17", f2s(2.8823261E17f));
#ifndef _WIN32
  // MSVC rounds this up to the next higher floating point number
  ASSERT_STREQ("7.038531E-26", f2s(7.038531E-26f));
#else
  ASSERT_STREQ("7.038531E-26", f2s(7.0385309E-26f));
#endif
  ASSERT_STREQ("9.223404E17", f2s(9.2234038E17f));
  ASSERT_STREQ("6.710887E7", f2s(6.7108872E7f));
  ASSERT_STREQ("1E-44", f2s(1.0E-44f));
  ASSERT_STREQ("2.816025E14", f2s(2.816025E14f));
  ASSERT_STREQ("9.223372E18", f2s(9.223372E18f));
  ASSERT_STREQ("1.5846086E29", f2s(1.5846085E29f));
  ASSERT_STREQ("1.1811161E19", f2s(1.1811161E19f));
  ASSERT_STREQ("5.368709E18", f2s(5.368709E18f));
  ASSERT_STREQ("4.6143166E18", f2s(4.6143165E18f));
  ASSERT_STREQ("7.812537E-3", f2s(0.007812537f));
  ASSERT_STREQ("1E-45", f2s(1.4E-45f));
  ASSERT_STREQ("1.18697725E20", f2s(1.18697724E20f));
  ASSERT_STREQ("1.00014165E-36", f2s(1.00014165E-36f));
  ASSERT_STREQ("2E2", f2s(200.0f));
  ASSERT_STREQ("3.3554432E7", f2s(3.3554432E7f));
}

TEST(F2sTest, OutputLength) {
  ASSERT_STREQ("1E0", f2s(1.0f)); // already tested in Basic
  ASSERT_STREQ("1.2E0", f2s(1.2f));
  ASSERT_STREQ("1.23E0", f2s(1.23f));
  ASSERT_STREQ("1.234E0", f2s(1.234f));
  ASSERT_STREQ("1.2345E0", f2s(1.2345f));
  ASSERT_STREQ("1.23456E0", f2s(1.23456f));
  ASSERT_STREQ("1.234567E0", f2s(1.234567f));
  ASSERT_STREQ("1.2345678E0", f2s(1.2345678f));
  ASSERT_STREQ("1.23456735E-36", f2s(1.23456735E-36f));
}
