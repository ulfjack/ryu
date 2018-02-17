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

import java.math.BigInteger;

/**
 * Prints a lookup table for the C version of Ryu.
 */
public final class PrintDoubleLookupTable {
  private static final int POS_TABLE_SIZE = 326;
  // The C version has two code paths, one of which requires an additional entry here.
  private static final int NEG_TABLE_SIZE = 291 + 1;

  private static final int POW5_BITCOUNT = 121; // max 127
  private static final int POW5_INV_BITCOUNT = 122; // max 127

  public static void main(String[] args) {
    BigInteger mask = BigInteger.valueOf(1).shiftLeft(64).subtract(BigInteger.ONE);

    System.out.println("#define POW5_INV_BITCOUNT " + POW5_INV_BITCOUNT);
    System.out.println("static uint64_t POW5_INV_SPLIT[" + NEG_TABLE_SIZE + "][2] = {");
    for (int i = 0; i < NEG_TABLE_SIZE; i++) {
      BigInteger pow = BigInteger.valueOf(5).pow(i);
      int pow5len = pow.bitLength();
      // We want floor(log_2 5^q) here, which is pow5len - 1.
      int j = pow5len - 1 + POW5_INV_BITCOUNT;
      BigInteger inv = BigInteger.ONE.shiftLeft(j).divide(pow).add(BigInteger.ONE);

      BigInteger pow5High = inv.shiftRight(64);
      BigInteger pow5Low = inv.and(mask);
      System.out.printf(" { %20su, %20su },", pow5Low, pow5High);
      if (i % 2 == 1) {
        System.out.println();
      }
    }
    System.out.println("};");

    System.out.println("#define POW5_BITCOUNT " + POW5_BITCOUNT);
    System.out.println("static uint64_t POW5_SPLIT[" + POS_TABLE_SIZE + "][2] = {");
    for (int i = 0; i < POS_TABLE_SIZE; i++) {
      BigInteger pow = BigInteger.valueOf(5).pow(i);
      int pow5len = pow.bitLength();
      BigInteger pow5High = pow.shiftRight(pow5len - POW5_BITCOUNT + 64);
      BigInteger pow5Low = pow.shiftRight(pow5len - POW5_BITCOUNT).and(mask);
      System.out.printf(" { %20su, %20su },", pow5Low, pow5High);
      if (i % 2 == 1) {
        System.out.println();
      }
    }
    System.out.println("};");
  }
}
