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

TEST(F2sTest, Basic) {
  ASSERT_STREQ("0E0", f2s(0.0));
  ASSERT_STREQ("-0E0", f2s(-0.0));
  ASSERT_STREQ("1E0", f2s(1.0));
  ASSERT_STREQ("-1E0", f2s(-1.0));
  ASSERT_STREQ("NaN", f2s(NAN));
  ASSERT_STREQ("Infinity", f2s(INFINITY));
  ASSERT_STREQ("-Infinity", f2s(-INFINITY));
}
