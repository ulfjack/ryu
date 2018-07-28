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

package info.adams.ryu.analysis;

import info.adams.random.MersenneTwister;
import info.adams.ryu.RyuDouble;

/**
 * Pseud-randomized tests the fast implementation of Ryu against the slow one.
 */
public class RandomizedDoubleComparison {
  public static void main(String[] args) {
    checkDoubleFastAgainstSlow();
  }

  private static void checkDoubleFastAgainstSlow() {
    MersenneTwister twister = new MersenneTwister(12345);
    // Warm up the JIT.
    for (int i = 0; i < 10000000; i++) {
      long r = twister.nextLong();
      double d = Double.longBitsToDouble(r);
      String expected = RyuDouble.doubleToString(d);
      String actual = SlowConversion.doubleToString(d);
      if (!expected.equals(actual)) {
        System.out.println(String.format("expected %s, but was %s", expected, actual));
        throw new RuntimeException(String.format("expected %s, but was %s", expected, actual));
      }
      if (i % 1000 == 0) {
        System.out.print(".");
      }
    }
    System.out.println();
  }
}
