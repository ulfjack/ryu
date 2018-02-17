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

TEST(F2sTest, RoundingModeEven) {
  ASSERT_STREQ("3.355445E7", f2s(3.355445E7f));
  ASSERT_STREQ("9E9", f2s(8.999999E9f));
  ASSERT_STREQ("3.436672E10", f2s(3.4366717E10f));
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
  ASSERT_STREQ("7.038531E-26", f2s(7.038531E-26f));
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

