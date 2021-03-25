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

package info.adams.ryu;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class RyuDoubleTest extends DoubleToStringTest {

    @Override
  String f(double f, RoundingMode roundingMode) {
    return RyuDouble.doubleToString(f, roundingMode);
  }

  @Test
  public void testNoExpNeg() {
    Assert.assertEquals("-1.234E-8",                  RyuDouble.doubleToString(-1.234e-8,  RoundingMode.ROUND_EVEN,  -3, 7));
    Assert.assertEquals("-0.00000001234",             RyuDouble.doubleToString(-1.234e-8,  RoundingMode.ROUND_EVEN,  -9, 7));
    Assert.assertEquals("-0.00000000000000000001234", RyuDouble.doubleToString(-1.234e-20, RoundingMode.ROUND_EVEN, -21, 7));
  }

  @Test
  public void testNoExpPos() {
    Assert.assertEquals("-1.234E8",                 RyuDouble.doubleToString(-1.234e8,  RoundingMode.ROUND_EVEN, -3,  7));
    Assert.assertEquals("-123400000.0",             RyuDouble.doubleToString(-1.234e8,  RoundingMode.ROUND_EVEN, -3,  9));
    Assert.assertEquals("-123400000000000000000.0", RyuDouble.doubleToString(-1.234e20, RoundingMode.ROUND_EVEN, -3, 21));
  }

}
