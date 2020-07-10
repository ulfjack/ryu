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

#include "ryu/ryu_generic_128.h"
#include "ryu/generic_128.h"
#include "third_party/gtest/gtest.h"

// We only test a few entries - we could test the full table instead, but should we?
static uint32_t EXACT_POW5_IDS[10] = {
  1, 10, 55, 56, 300, 1000, 2345, 3210, 4968 - 3, 4968 - 1
};

static uint64_t EXACT_POW5[10][4] = {
 {                    0u,                    0u,                    0u,    90071992547409920u },
 {                    0u,                    0u,                    0u,    83886080000000000u },
 {                    0u, 15708555500268290048u, 14699724349295723422u,   117549435082228750u },
 {                    0u,  5206161169240293376u,  4575641699882439235u,    73468396926392969u },
 {  2042133660145364371u,  9702060195405861314u,  6467325284806654637u,   107597969523956154u },
 { 15128847313296546509u, 11916317791371073891u,   788593023170869613u,   137108429762886488u },
 { 10998857860460266920u,   858411415306315808u, 12732466392391605111u,   136471991906002539u },
 {  5404652432687674341u, 18039986361557197657u,  2228774284272261859u,    94370442653226447u },
 { 15313487127299642753u,  9780770376910163681u, 15213531439620567348u,    93317108016191349u },
 {  7928436552078881485u,   723697829319983520u,   932817143438521969u,    72903990637649492u }
};

static uint32_t EXACT_INV_POW5_IDS[9] = {
  10, 55, 56, 300, 1000, 2345, 3210, 4897 - 3, 4897 - 1
};

static uint64_t EXACT_INV_POW5[10][4] = {
 { 13362655651931650467u,  3917988799323120213u,  9037289074543890586u,   123794003928538027u },
 {   983662216614650042u, 15516934687640009097u,  8839031818921249472u,    88342353238919216u },
 {  1573859546583440066u,  2691002611772552616u,  6763753280790178510u,   141347765182270746u },
 {  1607391579053635167u,   943946735193622172u, 10726301928680150504u,    96512915280967053u },
 {  7238603269427345471u, 17319296798264127544u, 14852913523241959878u,    75740009093741608u },
 {  2734564866961569744u, 13277212449690943834u, 17231454566843360565u,    76093223027199785u },
 {  5348945211244460332u, 14119936934335594321u, 15647307321253222579u,   110040743956546595u },
 {  2848579222248330872u, 15087265905644220040u,  4449739884766224405u,   100774177495370196u },
 {  1432572115632717323u,  9719393440895634811u,  3482057763655621045u,   128990947194073851u }
};

TEST(Generic128Test, generic_computePow5) {
  for (int i = 0; i < 10; i++) {
    uint64_t result[4];
    generic_computePow5(EXACT_POW5_IDS[i], result);
    ASSERT_EQ(EXACT_POW5[i][0], result[0]);
    ASSERT_EQ(EXACT_POW5[i][1], result[1]);
    ASSERT_EQ(EXACT_POW5[i][2], result[2]);
    ASSERT_EQ(EXACT_POW5[i][3], result[3]);
  }
}

TEST(Generic128Test, generic_computeInvPow5) {
  for (int i = 0; i < 9; i++) {
    uint64_t result[4];
    generic_computeInvPow5(EXACT_INV_POW5_IDS[i], result);
    ASSERT_EQ(EXACT_INV_POW5[i][0], result[0]);
    ASSERT_EQ(EXACT_INV_POW5[i][1], result[1]);
    ASSERT_EQ(EXACT_INV_POW5[i][2], result[2]);
    ASSERT_EQ(EXACT_INV_POW5[i][3], result[3]);
  }
}

TEST(Generic128Test, multipleOfPowerOf5) {
  ASSERT_EQ(true,  multipleOfPowerOf5(1, 0));
  ASSERT_EQ(false, multipleOfPowerOf5(1, 1));
  ASSERT_EQ(true,  multipleOfPowerOf5(5, 1));
  ASSERT_EQ(true,  multipleOfPowerOf5(25, 2));
  ASSERT_EQ(true,  multipleOfPowerOf5(75, 2));
  ASSERT_EQ(true,  multipleOfPowerOf5(50, 2));
  ASSERT_EQ(false, multipleOfPowerOf5(51, 2));
  ASSERT_EQ(false, multipleOfPowerOf5(75, 4));
}

TEST(Generic128Test, multipleOfPowerOf2) {
  ASSERT_EQ(true,  multipleOfPowerOf5(1, 0));
  ASSERT_EQ(false, multipleOfPowerOf5(1, 1));
  ASSERT_EQ(true,  multipleOfPowerOf2(2, 1));
  ASSERT_EQ(true,  multipleOfPowerOf2(4, 2));
  ASSERT_EQ(true,  multipleOfPowerOf2(8, 2));
  ASSERT_EQ(true,  multipleOfPowerOf2(12, 2));
  ASSERT_EQ(false, multipleOfPowerOf2(13, 2));
  ASSERT_EQ(false, multipleOfPowerOf2(8, 4));
}

TEST(Generic128Test, mulShift) {
  uint64_t m[4] = { 0, 0, 2, 0 };
  ASSERT_EQ(1u, mulShift(1, m, 129));
  ASSERT_EQ(12345u, mulShift(12345, m, 129));
}

TEST(Generic128Test, mulShiftHuge) {
  uint64_t m[4] = { 0, 0, 8, 0 };
  uint128_t f = (((uint128_t) 123) << 64) | 321;
  ASSERT_EQ(f, mulShift(f, m, 131));
}

TEST(Generic128Test, decimalLength) {
  ASSERT_EQ(1u, decimalLength(1));
  ASSERT_EQ(1u, decimalLength(9));
  ASSERT_EQ(2u, decimalLength(10));
  ASSERT_EQ(2u, decimalLength(99));
  ASSERT_EQ(3u, decimalLength(100));
  uint128_t tenPow38 = (((uint128_t) 5421010862427522170ull) << 64) | 687399551400673280ull;
  // 10^38 has 39 digits.
  ASSERT_EQ(39u, decimalLength(tenPow38));
}

TEST(Generic128Test, log10Pow2) {
  ASSERT_EQ(0u, log10Pow2(1));
  ASSERT_EQ(1u, log10Pow2(5));
  ASSERT_EQ(9864u, log10Pow2(1 << 15));
}

TEST(Generic128Test, log10Pow5) {
  ASSERT_EQ(0u, log10Pow5(1));
  ASSERT_EQ(1u, log10Pow5(2));
  ASSERT_EQ(2u, log10Pow5(3));
  ASSERT_EQ(22903u, log10Pow5(1 << 15));
}

TEST(Generic128Test, generic_to_chars) {
  char buffer[100];
  struct floating_decimal_128 v;
  v.mantissa = 12345;
  v.exponent = -2;
  v.sign = false;
  int index = generic_to_chars(v, buffer);
  buffer[index++] = 0;
  ASSERT_STREQ("1.2345E2", buffer);
}

TEST(Generic128Test, generic_to_chars_with_long_param) {
  char buffer[100];
  struct floating_decimal_128 v;
  v.mantissa = (((uint128_t) 5421010862427522170ull) << 64) | 687399551400673280ull;
  v.exponent = -20;
  v.sign = false;
  int index = generic_to_chars(v, buffer);
  buffer[index++] = 0;
  ASSERT_STREQ("1.00000000000000000000000000000000000000E18", buffer);
}

static char* f2s(float f) {
  const struct floating_decimal_128 fd = float_to_fd128(f);
  char* const result = (char*) malloc(25);
  const int index = generic_to_chars(fd, result);
  result[index] = '\0';
  return result;
}

#define ASSERT_F2S(a, b) { char* result = f2s(b); ASSERT_STREQ(a, result); free(result); } while (0);

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

TEST(Generic128Test, float_to_fd128) {
  ASSERT_F2S("0E0", 0.0);
  ASSERT_F2S("-0E0", -0.0);
  ASSERT_F2S("1E0", 1.0);
  ASSERT_F2S("-1E0", -1.0);
  ASSERT_F2S("NaN", NAN);
  ASSERT_F2S("Infinity", INFINITY);
  ASSERT_F2S("-Infinity", -INFINITY);
  ASSERT_F2S("1.1754944E-38", 1.1754944E-38f);
  ASSERT_F2S("3.4028235E38", int32Bits2Float(0x7f7fffff));
  ASSERT_F2S("1E-45", int32Bits2Float(1));
  ASSERT_F2S("3.355445E7", 3.355445E7f);
  ASSERT_F2S("9E9", 8.999999E9f);
  ASSERT_F2S("3.436672E10", 3.4366717E10f);
  ASSERT_F2S("3.0540412E5", 3.0540412E5f);
  ASSERT_F2S("8.0990312E3", 8.0990312E3f);
  // Pattern for the first test: 00111001100000000000000000000000
  ASSERT_F2S("2.4414062E-4", 2.4414062E-4f);
  ASSERT_F2S("2.4414062E-3", 2.4414062E-3f);
  ASSERT_F2S("4.3945312E-3", 4.3945312E-3f);
  ASSERT_F2S("6.3476562E-3", 6.3476562E-3f);
  ASSERT_F2S("4.7223665E21", 4.7223665E21f);
  ASSERT_F2S("8.388608E6", 8388608.0f);
  ASSERT_F2S("1.6777216E7", 1.6777216E7f);
  ASSERT_F2S("3.3554436E7", 3.3554436E7f);
  ASSERT_F2S("6.7131496E7", 6.7131496E7f);
  ASSERT_F2S("1.9310392E-38", 1.9310392E-38f);
  ASSERT_F2S("-2.47E-43", -2.47E-43f);
  ASSERT_F2S("1.993244E-38", 1.993244E-38f);
  ASSERT_F2S("4.1039004E3", 4103.9003f);
  ASSERT_F2S("5.3399997E9", 5.3399997E9f);
  ASSERT_F2S("6.0898E-39", 6.0898E-39f);
  ASSERT_F2S("1.0310042E-3", 0.0010310042f);
  ASSERT_F2S("2.882326E17", 2.8823261E17f);
#ifndef _WIN32
  // MSVC rounds this up to the next higher floating point number
  ASSERT_F2S("7.038531E-26", 7.038531E-26f);
#else
  ASSERT_F2S("7.038531E-26", 7.0385309E-26f);
#endif
  ASSERT_F2S("9.223404E17", 9.2234038E17f);
  ASSERT_F2S("6.710887E7", 6.7108872E7f);
  ASSERT_F2S("1E-44", 1.0E-44f);
  ASSERT_F2S("2.816025E14", 2.816025E14f);
  ASSERT_F2S("9.223372E18", 9.223372E18f);
  ASSERT_F2S("1.5846086E29", 1.5846085E29f);
  ASSERT_F2S("1.1811161E19", 1.1811161E19f);
  ASSERT_F2S("5.368709E18", 5.368709E18f);
  ASSERT_F2S("4.6143166E18", 4.6143165E18f);
  ASSERT_F2S("7.812537E-3", 0.007812537f);
  ASSERT_F2S("1E-45", 1.4E-45f);
  ASSERT_F2S("1.18697725E20", 1.18697724E20f);
  ASSERT_F2S("1.00014165E-36", 1.00014165E-36f);
  ASSERT_F2S("2E2", 200.0f);
  ASSERT_F2S("3.3554432E7", 3.3554432E7f);
  ASSERT_F2S("1E0", 1.0f); // already tested in Basic
  ASSERT_F2S("1.2E0", 1.2f);
  ASSERT_F2S("1.23E0", 1.23f);
  ASSERT_F2S("1.234E0", 1.234f);
  ASSERT_F2S("1.2345E0", 1.2345f);
  ASSERT_F2S("1.23456E0", 1.23456f);
  ASSERT_F2S("1.234567E0", 1.234567f);
  ASSERT_F2S("1.2345678E0", 1.2345678f);
  ASSERT_F2S("1.23456735E-36", 1.23456735E-36f);
}

TEST(Generic128Test, direct_double_to_fd128) {
  const struct floating_decimal_128 v = double_to_fd128(4.708356024711512E18);
  ASSERT_EQ(false, v.sign);
  ASSERT_EQ(3, v.exponent);
  ASSERT_EQ(4708356024711512ull, v.mantissa);
}

static char* d2s(double d) {
  const struct floating_decimal_128 v = double_to_fd128(d);
  char* const result = (char*) malloc(25);
  const int index = generic_to_chars(v, result);
  result[index] = '\0';
  return result;
}

#define ASSERT_D2S(a, b) { char* result = d2s(b); ASSERT_STREQ(a, result); free(result); } while (0);

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

TEST(Generic128Test, double_to_fd128) {
  ASSERT_D2S("0E0", 0.0);
  ASSERT_D2S("-0E0", -0.0);
  ASSERT_D2S("1E0", 1.0);
  ASSERT_D2S("-1E0", -1.0);
  ASSERT_D2S("NaN", NAN);
  ASSERT_D2S("Infinity", INFINITY);
  ASSERT_D2S("-Infinity", -INFINITY);
  ASSERT_D2S("2.2250738585072014E-308", 2.2250738585072014E-308);
  ASSERT_D2S("1.7976931348623157E308", int64Bits2Double(0x7fefffffffffffff));
  ASSERT_D2S("5E-324", int64Bits2Double(1));
  ASSERT_D2S("2.9802322387695312E-8", 2.98023223876953125E-8);
  ASSERT_D2S("-2.109808898695963E16", -2.109808898695963E16);
  ASSERT_D2S("4.940656E-318", 4.940656E-318);
  ASSERT_D2S("1.18575755E-316", 1.18575755E-316);
  ASSERT_D2S("2.989102097996E-312", 2.989102097996E-312);
  ASSERT_D2S("9.0608011534336E15", 9.0608011534336E15);
  ASSERT_D2S("4.708356024711512E18", 4.708356024711512E18);
  ASSERT_D2S("9.409340012568248E18", 9.409340012568248E18);
  ASSERT_D2S("1.2345678E0", 1.2345678);
  ASSERT_D2S("5.764607523034235E39", int64Bits2Double(0x4830F0CF064DD592));
  ASSERT_D2S("1.152921504606847E40", int64Bits2Double(0x4840F0CF064DD592));
  ASSERT_D2S("2.305843009213694E40", int64Bits2Double(0x4850F0CF064DD592));

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

static char* l2s(long double d) {
  const struct floating_decimal_128 v = long_double_to_fd128(d);
  char* const result = (char*) malloc(60);
  const int index = generic_to_chars(v, result);
  result[index] = '\0';
  return result;
}

#define ASSERT_L2S(a, b) { char* result = l2s(b); ASSERT_STREQ(a, result); free(result); } while (0);

TEST(Generic128Test, long_double_to_fd128) {
  ASSERT_L2S("0E0", 0.0);
  ASSERT_L2S("-0E0", -0.0);
  ASSERT_L2S("1E0", 1.0);
  ASSERT_L2S("-1E0", -1.0);
  ASSERT_L2S("NaN", NAN);
  ASSERT_L2S("Infinity", INFINITY);
  ASSERT_L2S("-Infinity", -INFINITY);

  ASSERT_L2S("2.2250738585072014E-308", 2.2250738585072014E-308L);
  ASSERT_L2S("2.98023223876953125E-8", 2.98023223876953125E-8L);
  ASSERT_L2S("-2.109808898695963E16", -2.109808898695963E16L);
  ASSERT_L2S("4.940656E-318", 4.940656E-318L);
  ASSERT_L2S("1.18575755E-316", 1.18575755E-316L);
  ASSERT_L2S("2.989102097996E-312", 2.989102097996E-312L);
  ASSERT_L2S("9.0608011534336E15", 9.0608011534336E15L);
  ASSERT_L2S("4.708356024711512E18", 4.708356024711512E18L);
  ASSERT_L2S("9.409340012568248E18", 9.409340012568248E18L);
  ASSERT_L2S("1.2345678E0", 1.2345678L);
}

TEST(Generic128Test, regression_test_long_double) {
  // The binary 80-bit representation of this number has a mantissa that is a
  // one followed by zeros. This is a special case, because the next lower
  // number is closer to X than the next higher number, so it is possible that
  // we need to print more digits.
  // Before this test was added, the code incorrectly checked for an all-zeroes
  // mantissa in this case - the 80-bit format has an *explicit* leading one,
  // and the code did not take that into account.
  long double l = 1.10169395793497080013e-4927L;
  ASSERT_L2S("1.10169395793497080013E-4927", l);
  // Also check that the next higher and next lower number have *different*
  // decimal representations.
  ASSERT_L2S("1.1016939579349708003E-4927", nextafterl(l, INFINITY));
  ASSERT_L2S("1.1016939579349708001E-4927", nextafterl(l, -INFINITY));
}
