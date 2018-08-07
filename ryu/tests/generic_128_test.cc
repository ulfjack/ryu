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
  ASSERT_EQ(1, mulShift(1, m, 129));
  ASSERT_EQ(12345, mulShift(12345, m, 129));
}

TEST(Generic128Test, mulShiftHuge) {
  uint64_t m[4] = { 0, 0, 8, 0 };
  uint128_t f = (((uint128_t) 123) << 64) | 321;
  ASSERT_EQ(f, mulShift(f, m, 131));
}

TEST(Generic128Test, decimalLength) {
  ASSERT_EQ(1, decimalLength(1));
  ASSERT_EQ(1, decimalLength(9));
  ASSERT_EQ(2, decimalLength(10));
  ASSERT_EQ(2, decimalLength(99));
  ASSERT_EQ(3, decimalLength(100));
  uint128_t tenPow38 = (((uint128_t) 5421010862427522170ull) << 64) | 687399551400673280ull;
  // 10^38 has 39 digits.
  ASSERT_EQ(39, decimalLength(tenPow38));
}

TEST(Generic128Test, log10Pow2) {
  ASSERT_EQ(0, log10Pow2(1));
  ASSERT_EQ(1, log10Pow2(5));
  ASSERT_EQ(9864, log10Pow2(1 << 15));
}

TEST(Generic128Test, log10Pow5) {
  ASSERT_EQ(0, log10Pow5(1));
  ASSERT_EQ(1, log10Pow5(2));
  ASSERT_EQ(2, log10Pow5(3));
  ASSERT_EQ(22903, log10Pow5(1 << 15));
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

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

TEST(Generic128Test, float_to_fd128) {
  ASSERT_STREQ("0E0", f2s(0.0));
  ASSERT_STREQ("-0E0", f2s(-0.0));
  ASSERT_STREQ("1E0", f2s(1.0));
  ASSERT_STREQ("-1E0", f2s(-1.0));
  ASSERT_STREQ("NaN", f2s(NAN));
  ASSERT_STREQ("Infinity", f2s(INFINITY));
  ASSERT_STREQ("-Infinity", f2s(-INFINITY));
  ASSERT_STREQ("1.1754944E-38", f2s(1.1754944E-38f));
  ASSERT_STREQ("3.4028235E38", f2s(int32Bits2Float(0x7f7fffff)));
  ASSERT_STREQ("1E-45", f2s(int32Bits2Float(1)));
  ASSERT_STREQ("3.355445E7", f2s(3.355445E7f));
  ASSERT_STREQ("9E9", f2s(8.999999E9f));
  ASSERT_STREQ("3.436672E10", f2s(3.4366717E10f));
  ASSERT_STREQ("3.0540412E5", f2s(3.0540412E5f));
  ASSERT_STREQ("8.0990312E3", f2s(8.0990312E3f));
  // Pattern for the first test: 00111001100000000000000000000000
  ASSERT_STREQ("2.4414062E-4", f2s(2.4414062E-4f));
  ASSERT_STREQ("2.4414062E-3", f2s(2.4414062E-3f));
  ASSERT_STREQ("4.3945312E-3", f2s(4.3945312E-3f));
  ASSERT_STREQ("6.3476562E-3", f2s(6.3476562E-3f));
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

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

TEST(Generic128Test, double_to_fd128) {
  ASSERT_STREQ("0E0", d2s(0.0));
  ASSERT_STREQ("-0E0", d2s(-0.0));
  ASSERT_STREQ("1E0", d2s(1.0));
  ASSERT_STREQ("-1E0", d2s(-1.0));
  ASSERT_STREQ("NaN", d2s(NAN));
  ASSERT_STREQ("Infinity", d2s(INFINITY));
  ASSERT_STREQ("-Infinity", d2s(-INFINITY));
  ASSERT_STREQ("2.2250738585072014E-308", d2s(2.2250738585072014E-308));
  ASSERT_STREQ("1.7976931348623157E308", d2s(int64Bits2Double(0x7fefffffffffffff)));
  ASSERT_STREQ("5E-324", d2s(int64Bits2Double(1)));
  ASSERT_STREQ("2.9802322387695312E-8", d2s(2.98023223876953125E-8));
  ASSERT_STREQ("-2.109808898695963E16", d2s(-2.109808898695963E16));
  ASSERT_STREQ("4.940656E-318", d2s(4.940656E-318));
  ASSERT_STREQ("1.18575755E-316", d2s(1.18575755E-316));
  ASSERT_STREQ("2.989102097996E-312", d2s(2.989102097996E-312));
  ASSERT_STREQ("9.0608011534336E15", d2s(9.0608011534336E15));
  ASSERT_STREQ("4.708356024711512E18", d2s(4.708356024711512E18));
  ASSERT_STREQ("9.409340012568248E18", d2s(9.409340012568248E18));
  ASSERT_STREQ("1.2345678E0", d2s(1.2345678));
  ASSERT_STREQ("5.764607523034235E39", d2s(int64Bits2Double(0x4830F0CF064DD592)));
  ASSERT_STREQ("1.152921504606847E40", d2s(int64Bits2Double(0x4840F0CF064DD592)));
  ASSERT_STREQ("2.305843009213694E40", d2s(int64Bits2Double(0x4850F0CF064DD592)));

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

static char* l2s(long double d) {
  const struct floating_decimal_128 v = long_double_to_fd128(d);
  char* const result = (char*) malloc(60);
  const int index = generic_to_chars(v, result);
  result[index] = '\0';
  return result;
}

TEST(Generic128Test, long_double_to_fd128) {
  ASSERT_STREQ("0E0", l2s(0.0));
  ASSERT_STREQ("-0E0", l2s(-0.0));
  ASSERT_STREQ("1E0", l2s(1.0));
  ASSERT_STREQ("-1E0", l2s(-1.0));
  ASSERT_STREQ("NaN", l2s(NAN));
  ASSERT_STREQ("Infinity", l2s(INFINITY));
  ASSERT_STREQ("-Infinity", l2s(-INFINITY));

  ASSERT_STREQ("2.2250738585072014E-308", l2s(2.2250738585072014E-308L));
  ASSERT_STREQ("2.98023223876953125E-8", l2s(2.98023223876953125E-8L));
  ASSERT_STREQ("-2.109808898695963E16", l2s(-2.109808898695963E16L));
  ASSERT_STREQ("4.940656E-318", l2s(4.940656E-318L));
  ASSERT_STREQ("1.18575755E-316", l2s(1.18575755E-316L));
  ASSERT_STREQ("2.989102097996E-312", l2s(2.989102097996E-312L));
  ASSERT_STREQ("9.0608011534336E15", l2s(9.0608011534336E15L));
  ASSERT_STREQ("4.708356024711512E18", l2s(4.708356024711512E18L));
  ASSERT_STREQ("9.409340012568248E18", l2s(9.409340012568248E18L));
  ASSERT_STREQ("1.2345678E0", l2s(1.2345678L));
}
