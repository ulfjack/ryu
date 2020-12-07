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

#include "ryu/d2s_intrinsics.h"
#include "third_party/gtest/gtest.h"

TEST(D2sIntrinsicsTest, mod1e9) {
  EXPECT_EQ(0u, mod1e9(0));
  EXPECT_EQ(1u, mod1e9(1));
  EXPECT_EQ(2u, mod1e9(2));
  EXPECT_EQ(10u, mod1e9(10));
  EXPECT_EQ(100u, mod1e9(100));
  EXPECT_EQ(1000u, mod1e9(1000));
  EXPECT_EQ(10000u, mod1e9(10000));
  EXPECT_EQ(100000u, mod1e9(100000));
  EXPECT_EQ(1000000u, mod1e9(1000000));
  EXPECT_EQ(10000000u, mod1e9(10000000));
  EXPECT_EQ(100000000u, mod1e9(100000000));
  EXPECT_EQ(0u, mod1e9(1000000000));
  EXPECT_EQ(0u, mod1e9(2000000000));
  EXPECT_EQ(1u, mod1e9(1000000001));
  EXPECT_EQ(1234u, mod1e9(1000001234));
  EXPECT_EQ(123456789u, mod1e9(12345123456789ull));
  EXPECT_EQ(123456789u, mod1e9(123456789123456789ull));
}

TEST(D2sIntrinsicsTest, shiftRight128) {
  EXPECT_EQ(0x100000000ull, shiftright128(0x1ull, 0x1ull, 32));
}

TEST(D2sIntrinsicsTest, pow5Factor) {
  EXPECT_EQ(0u, pow5Factor(1ull));
  EXPECT_EQ(0u, pow5Factor(2ull));
  EXPECT_EQ(0u, pow5Factor(3ull));
  EXPECT_EQ(0u, pow5Factor(4ull));
  EXPECT_EQ(1u, pow5Factor(5ull));
  EXPECT_EQ(0u, pow5Factor(6ull));
  EXPECT_EQ(0u, pow5Factor(7ull));
  EXPECT_EQ(0u, pow5Factor(8ull));
  EXPECT_EQ(0u, pow5Factor(9ull));
  EXPECT_EQ(1u, pow5Factor(10ull));

  EXPECT_EQ(0u, pow5Factor(12ull));
  EXPECT_EQ(0u, pow5Factor(14ull));
  EXPECT_EQ(0u, pow5Factor(16ull));
  EXPECT_EQ(0u, pow5Factor(18ull));
  EXPECT_EQ(1u, pow5Factor(20ull));

  EXPECT_EQ(2u, pow5Factor(5*5ull));
  EXPECT_EQ(3u, pow5Factor(5*5*5ull));
  EXPECT_EQ(4u, pow5Factor(5*5*5*5ull));
  EXPECT_EQ(5u, pow5Factor(5*5*5*5*5ull));
  EXPECT_EQ(6u, pow5Factor(5*5*5*5*5*5ull));
  EXPECT_EQ(7u, pow5Factor(5*5*5*5*5*5*5ull));
  EXPECT_EQ(8u, pow5Factor(5*5*5*5*5*5*5*5ull));
  EXPECT_EQ(9u, pow5Factor(5*5*5*5*5*5*5*5*5ull));
  EXPECT_EQ(10u, pow5Factor(5*5*5*5*5*5*5*5*5*5ull));

  EXPECT_EQ(0u, pow5Factor(42ull));
  EXPECT_EQ(1u, pow5Factor(42*5ull));
  EXPECT_EQ(2u, pow5Factor(42*5*5ull));
  EXPECT_EQ(3u, pow5Factor(42*5*5*5ull));
  EXPECT_EQ(4u, pow5Factor(42*5*5*5*5ull));
  EXPECT_EQ(5u, pow5Factor(42*5*5*5*5*5ull));

  EXPECT_EQ(27u, pow5Factor(7450580596923828125ull)); // 5^27, largest power of 5 < 2^64.
  EXPECT_EQ(1u, pow5Factor(18446744073709551615ull)); // 2^64 - 1, largest multiple of 5 < 2^64.
  EXPECT_EQ(0u, pow5Factor(18446744073709551614ull)); // 2^64 - 2, largest non-multiple of 5 < 2^64.
}
