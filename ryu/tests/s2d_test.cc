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

TEST(S2dTest, Basic) {
  EXPECT_EQ(0.0, s2d("0"));
  EXPECT_EQ(-0.0, s2d("-0"));
  EXPECT_EQ(1.0, s2d("1"));
  EXPECT_EQ(2.0, s2d("2"));
  EXPECT_EQ(123456789.0, s2d("123456789"));
  EXPECT_EQ(123.456, s2d("123.456"));
  EXPECT_EQ(123.456, s2d("123456e-3"));
  EXPECT_EQ(123.456, s2d("1234.56e-1"));
  EXPECT_EQ(1.453, s2d("1.453"));
}
