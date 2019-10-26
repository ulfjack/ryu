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

static double ieeeParts2Double(const bool sign, const uint32_t ieeeExponent, const uint64_t ieeeMantissa) {
  assert(ieeeExponent <= 2047);
  assert(ieeeMantissa <= ((uint64_t)1 << 53) - 1);
  return int64Bits2Double(((uint64_t)sign << 63) | ((uint64_t)ieeeExponent << 52) | ieeeMantissa);
}

#define ASSERT_D2S(a, b) { char* result = d2s(b); ASSERT_STREQ(a, result); free(result); } while (0);

TEST(D2sTest, Basic) {
  ASSERT_D2S("0E0", 0.0);
  ASSERT_D2S("-0E0", -0.0);
  ASSERT_D2S("1E0", 1.0);
  ASSERT_D2S("-1E0", -1.0);
  ASSERT_D2S("NaN", NAN);
  ASSERT_D2S("Infinity", INFINITY);
  ASSERT_D2S("-Infinity", -INFINITY);
}

TEST(D2sTest, SwitchToSubnormal) {
  ASSERT_D2S("2.2250738585072014E-308", 2.2250738585072014E-308);
}

TEST(D2sTest, MinAndMax) {
  ASSERT_D2S("1.7976931348623157E308", int64Bits2Double(0x7fefffffffffffff));
  ASSERT_D2S("5E-324", int64Bits2Double(1));
}

TEST(D2sTest, LotsOfTrailingZeros) {
  ASSERT_D2S("2.9802322387695312E-8", 2.98023223876953125E-8);
}

TEST(D2sTest, Regression) {
  ASSERT_D2S("-2.109808898695963E16", -2.109808898695963E16);
  ASSERT_D2S("4.940656E-318", 4.940656E-318);
  ASSERT_D2S("1.18575755E-316", 1.18575755E-316);
  ASSERT_D2S("2.989102097996E-312", 2.989102097996E-312);
  ASSERT_D2S("9.0608011534336E15", 9.0608011534336E15);
  ASSERT_D2S("4.708356024711512E18", 4.708356024711512E18);
  ASSERT_D2S("9.409340012568248E18", 9.409340012568248E18);
  ASSERT_D2S("1.2345678E0", 1.2345678);
}

TEST(D2sTest, LooksLikePow5) {
  // These numbers have a mantissa that is a multiple of the largest power of 5 that fits,
  // and an exponent that causes the computation for q to result in 22, which is a corner
  // case for Ryu.
  ASSERT_D2S("5.764607523034235E39", int64Bits2Double(0x4830F0CF064DD592));
  ASSERT_D2S("1.152921504606847E40", int64Bits2Double(0x4840F0CF064DD592));
  ASSERT_D2S("2.305843009213694E40", int64Bits2Double(0x4850F0CF064DD592));
}

TEST(D2sTest, OutputLength) {
  ASSERT_D2S("1E0", 1); // already tested in Basic
  ASSERT_D2S("1.2E0", 1.2);
  ASSERT_D2S("1.23E0", 1.23);
  ASSERT_D2S("1.234E0", 1.234);
  ASSERT_D2S("1.2345E0", 1.2345);
  ASSERT_D2S("1.23456E0", 1.23456);
  ASSERT_D2S("1.234567E0", 1.234567);
  ASSERT_D2S("1.2345678E0", 1.2345678); // already tested in Regression
  ASSERT_D2S("1.23456789E0", 1.23456789);
  ASSERT_D2S("1.234567895E0", 1.234567895); // 1.234567890 would be trimmed
  ASSERT_D2S("1.2345678901E0", 1.2345678901);
  ASSERT_D2S("1.23456789012E0", 1.23456789012);
  ASSERT_D2S("1.234567890123E0", 1.234567890123);
  ASSERT_D2S("1.2345678901234E0", 1.2345678901234);
  ASSERT_D2S("1.23456789012345E0", 1.23456789012345);
  ASSERT_D2S("1.234567890123456E0", 1.234567890123456);
  ASSERT_D2S("1.2345678901234567E0", 1.2345678901234567);

  // Test 32-bit chunking
  ASSERT_D2S("4.294967294E0", 4.294967294); // 2^32 - 2
  ASSERT_D2S("4.294967295E0", 4.294967295); // 2^32 - 1
  ASSERT_D2S("4.294967296E0", 4.294967296); // 2^32
  ASSERT_D2S("4.294967297E0", 4.294967297); // 2^32 + 1
  ASSERT_D2S("4.294967298E0", 4.294967298); // 2^32 + 2
}

// Test min, max shift values in shiftright128
TEST(D2sTest, MinMaxShift) {
  const uint64_t maxMantissa = ((uint64_t)1 << 53) - 1;

  // 32-bit opt-size=0:  49 <= dist <= 50
  // 32-bit opt-size=1:  30 <= dist <= 50
  // 64-bit opt-size=0:  50 <= dist <= 50
  // 64-bit opt-size=1:  30 <= dist <= 50
  ASSERT_D2S("1.7800590868057611E-307", ieeeParts2Double(false, 4, 0));
  // 32-bit opt-size=0:  49 <= dist <= 49
  // 32-bit opt-size=1:  28 <= dist <= 49
  // 64-bit opt-size=0:  50 <= dist <= 50
  // 64-bit opt-size=1:  28 <= dist <= 50
  ASSERT_D2S("2.8480945388892175E-306", ieeeParts2Double(false, 6, maxMantissa));
  // 32-bit opt-size=0:  52 <= dist <= 53
  // 32-bit opt-size=1:   2 <= dist <= 53
  // 64-bit opt-size=0:  53 <= dist <= 53
  // 64-bit opt-size=1:   2 <= dist <= 53
  ASSERT_D2S("2.446494580089078E-296", ieeeParts2Double(false, 41, 0));
  // 32-bit opt-size=0:  52 <= dist <= 52
  // 32-bit opt-size=1:   2 <= dist <= 52
  // 64-bit opt-size=0:  53 <= dist <= 53
  // 64-bit opt-size=1:   2 <= dist <= 53
  ASSERT_D2S("4.8929891601781557E-296", ieeeParts2Double(false, 40, maxMantissa));

  // 32-bit opt-size=0:  57 <= dist <= 58
  // 32-bit opt-size=1:  57 <= dist <= 58
  // 64-bit opt-size=0:  58 <= dist <= 58
  // 64-bit opt-size=1:  58 <= dist <= 58
  ASSERT_D2S("1.8014398509481984E16", ieeeParts2Double(false, 1077, 0));
  // 32-bit opt-size=0:  57 <= dist <= 57
  // 32-bit opt-size=1:  57 <= dist <= 57
  // 64-bit opt-size=0:  58 <= dist <= 58
  // 64-bit opt-size=1:  58 <= dist <= 58
  ASSERT_D2S("3.6028797018963964E16", ieeeParts2Double(false, 1076, maxMantissa));
  // 32-bit opt-size=0:  51 <= dist <= 52
  // 32-bit opt-size=1:  51 <= dist <= 59
  // 64-bit opt-size=0:  52 <= dist <= 52
  // 64-bit opt-size=1:  52 <= dist <= 59
  ASSERT_D2S("2.900835519859558E-216", ieeeParts2Double(false, 307, 0));
  // 32-bit opt-size=0:  51 <= dist <= 51
  // 32-bit opt-size=1:  51 <= dist <= 59
  // 64-bit opt-size=0:  52 <= dist <= 52
  // 64-bit opt-size=1:  52 <= dist <= 59
  ASSERT_D2S("5.801671039719115E-216", ieeeParts2Double(false, 306, maxMantissa));

  // https://github.com/ulfjack/ryu/commit/19e44d16d80236f5de25800f56d82606d1be00b9#commitcomment-30146483
  // 32-bit opt-size=0:  49 <= dist <= 49
  // 32-bit opt-size=1:  44 <= dist <= 49
  // 64-bit opt-size=0:  50 <= dist <= 50
  // 64-bit opt-size=1:  44 <= dist <= 50
  ASSERT_D2S("3.196104012172126E-27", ieeeParts2Double(false, 934, 0x000FA7161A4D6E0Cu));
}

TEST(D2sTest, SmallIntegers) {
  ASSERT_D2S("9.007199254740991E15", 9007199254740991.0); // 2^53-1
  ASSERT_D2S("9.007199254740992E15", 9007199254740992.0); // 2^53

  ASSERT_D2S("1E0", 1.0e+0);
  ASSERT_D2S("1.2E1", 1.2e+1);
  ASSERT_D2S("1.23E2", 1.23e+2);
  ASSERT_D2S("1.234E3", 1.234e+3);
  ASSERT_D2S("1.2345E4", 1.2345e+4);
  ASSERT_D2S("1.23456E5", 1.23456e+5);
  ASSERT_D2S("1.234567E6", 1.234567e+6);
  ASSERT_D2S("1.2345678E7", 1.2345678e+7);
  ASSERT_D2S("1.23456789E8", 1.23456789e+8);
  ASSERT_D2S("1.23456789E9", 1.23456789e+9);
  ASSERT_D2S("1.234567895E9", 1.234567895e+9);
  ASSERT_D2S("1.2345678901E10", 1.2345678901e+10);
  ASSERT_D2S("1.23456789012E11", 1.23456789012e+11);
  ASSERT_D2S("1.234567890123E12", 1.234567890123e+12);
  ASSERT_D2S("1.2345678901234E13", 1.2345678901234e+13);
  ASSERT_D2S("1.23456789012345E14", 1.23456789012345e+14);
  ASSERT_D2S("1.234567890123456E15", 1.234567890123456e+15);

  // 10^i
  ASSERT_D2S("1E0", 1.0e+0);
  ASSERT_D2S("1E1", 1.0e+1);
  ASSERT_D2S("1E2", 1.0e+2);
  ASSERT_D2S("1E3", 1.0e+3);
  ASSERT_D2S("1E4", 1.0e+4);
  ASSERT_D2S("1E5", 1.0e+5);
  ASSERT_D2S("1E6", 1.0e+6);
  ASSERT_D2S("1E7", 1.0e+7);
  ASSERT_D2S("1E8", 1.0e+8);
  ASSERT_D2S("1E9", 1.0e+9);
  ASSERT_D2S("1E10", 1.0e+10);
  ASSERT_D2S("1E11", 1.0e+11);
  ASSERT_D2S("1E12", 1.0e+12);
  ASSERT_D2S("1E13", 1.0e+13);
  ASSERT_D2S("1E14", 1.0e+14);
  ASSERT_D2S("1E15", 1.0e+15);

  // 10^15 + 10^i
  ASSERT_D2S("1.000000000000001E15", 1.0e+15 + 1.0e+0);
  ASSERT_D2S("1.00000000000001E15", 1.0e+15 + 1.0e+1);
  ASSERT_D2S("1.0000000000001E15", 1.0e+15 + 1.0e+2);
  ASSERT_D2S("1.000000000001E15", 1.0e+15 + 1.0e+3);
  ASSERT_D2S("1.00000000001E15", 1.0e+15 + 1.0e+4);
  ASSERT_D2S("1.0000000001E15", 1.0e+15 + 1.0e+5);
  ASSERT_D2S("1.000000001E15", 1.0e+15 + 1.0e+6);
  ASSERT_D2S("1.00000001E15", 1.0e+15 + 1.0e+7);
  ASSERT_D2S("1.0000001E15", 1.0e+15 + 1.0e+8);
  ASSERT_D2S("1.000001E15", 1.0e+15 + 1.0e+9);
  ASSERT_D2S("1.00001E15", 1.0e+15 + 1.0e+10);
  ASSERT_D2S("1.0001E15", 1.0e+15 + 1.0e+11);
  ASSERT_D2S("1.001E15", 1.0e+15 + 1.0e+12);
  ASSERT_D2S("1.01E15", 1.0e+15 + 1.0e+13);
  ASSERT_D2S("1.1E15", 1.0e+15 + 1.0e+14);

  // Largest power of 2 <= 10^(i+1)
  ASSERT_D2S("8E0", 8.0);
  ASSERT_D2S("6.4E1", 64.0);
  ASSERT_D2S("5.12E2", 512.0);
  ASSERT_D2S("8.192E3", 8192.0);
  ASSERT_D2S("6.5536E4", 65536.0);
  ASSERT_D2S("5.24288E5", 524288.0);
  ASSERT_D2S("8.388608E6", 8388608.0);
  ASSERT_D2S("6.7108864E7", 67108864.0);
  ASSERT_D2S("5.36870912E8", 536870912.0);
  ASSERT_D2S("8.589934592E9", 8589934592.0);
  ASSERT_D2S("6.8719476736E10", 68719476736.0);
  ASSERT_D2S("5.49755813888E11", 549755813888.0);
  ASSERT_D2S("8.796093022208E12", 8796093022208.0);
  ASSERT_D2S("7.0368744177664E13", 70368744177664.0);
  ASSERT_D2S("5.62949953421312E14", 562949953421312.0);
  ASSERT_D2S("9.007199254740992E15", 9007199254740992.0);

  // 1000 * (Largest power of 2 <= 10^(i+1))
  ASSERT_D2S("8E3", 8.0e+3);
  ASSERT_D2S("6.4E4", 64.0e+3);
  ASSERT_D2S("5.12E5", 512.0e+3);
  ASSERT_D2S("8.192E6", 8192.0e+3);
  ASSERT_D2S("6.5536E7", 65536.0e+3);
  ASSERT_D2S("5.24288E8", 524288.0e+3);
  ASSERT_D2S("8.388608E9", 8388608.0e+3);
  ASSERT_D2S("6.7108864E10", 67108864.0e+3);
  ASSERT_D2S("5.36870912E11", 536870912.0e+3);
  ASSERT_D2S("8.589934592E12", 8589934592.0e+3);
  ASSERT_D2S("6.8719476736E13", 68719476736.0e+3);
  ASSERT_D2S("5.49755813888E14", 549755813888.0e+3);
  ASSERT_D2S("8.796093022208E15", 8796093022208.0e+3);
}
