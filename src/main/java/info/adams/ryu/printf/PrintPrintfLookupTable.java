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

package info.adams.ryu.printf;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Prints a lookup table for the C version of Ryu.
 */
public final class PrintPrintfLookupTable {
  private static final int POW10_ADDITIONAL_BITS = 120;
  // Multiplier < 10^9 * 2^BITS
  // BITS >= pow10BitsForIndex(indexForExponent(e2)) + POW10_ADDITIONAL_BITS - e2
  //      >= 16 * [(e2 + 15) / 16] + POW10_ADDITIONAL_BITS - e2
  //      >= (e2 + 15) + POW10_ADDITIONAL_BITS - e2
  //      >= 15 + POW10_ADDITIONAL_BITS
  private static final int BITS = POW10_ADDITIONAL_BITS + 16;
  private static final int TABLE_SIZE = 64;

  private static final int ADDITIONAL_BITS_2 = 120; // max 192 - 32 - 16 = 144
  private static final int TABLE_SIZE_2 = 68;

  private static final BigInteger mask = BigInteger.valueOf(1).shiftLeft(64).subtract(BigInteger.ONE);

  public static void main(String[] args) {
    printTables();
    printInverseTables();
  }

  private static void printTables() {
    int sum = 0;
    for (int idx = 0; idx < TABLE_SIZE; idx++) {
      System.out.print(sum + ", ");
      int len = lengthForIndex(idx);
      sum += len;
    }
    System.out.println();
    int totalLength = 0;
    for (int idx = 0; idx < TABLE_SIZE; idx++) {
      totalLength += lengthForIndex(idx);
    }
    System.out.println("static uint64_t POW10_SPLIT[" + totalLength + "][3] = {");
    for (int idx = 0; idx < TABLE_SIZE; idx++) {
      int pow10bits = pow10BitsForIndex(idx);
      int len = lengthForIndex(idx);
      for (int i = 0; i < len; i++) {
        BigInteger v = BigInteger.valueOf(1).shiftLeft(pow10bits)
            .divide(BigInteger.TEN.pow(9 * i))
            .add(BigInteger.ONE)
            .mod(BigInteger.TEN.pow(9).shiftLeft(BITS));
        if (v.bitLength() > 192) {
          throw new IllegalStateException("" + v.bitLength());
        }
        System.out.printf(" { ");
        for (int j = 0; j < 3; j++) {
          if (j != 0) {
            System.out.print(", ");
          }
          BigInteger bits = v.shiftRight(64 * j).and(mask);
          System.out.printf("%20su", bits);
        }
        System.out.printf(" },\n");
      }
    }
    System.out.println("};");
  }

  private static void printInverseTables() {
    BigInteger lowerCutoff = BigInteger.ONE.shiftLeft(54 + 8);
    int[] min = new int[TABLE_SIZE_2];
    int[] max = new int[TABLE_SIZE_2];
    int[] offset = new int[TABLE_SIZE_2];
    List<BigInteger> data = new ArrayList<>();
    Arrays.fill(min, -1);
    Arrays.fill(max, -1);
    for (int idx = 0; idx < TABLE_SIZE_2; idx++) {
      int len = lengthForIndex2(idx);
      int k = ADDITIONAL_BITS_2;
      offset[idx] = data.size();
      for (int i = 0; ; i++) {
        BigInteger v = BigInteger.TEN.pow(9 * (i + 1))
            .shiftRight(-(k - 16 * idx))
            .mod(BigInteger.TEN.pow(9).shiftLeft(k + 16));
        if (v.bitLength() > 192) { // 32 + 74 + 16 = 122
          throw new IllegalStateException("" + v.bitLength());
        }
        if (min[idx] == -1 && v.multiply(lowerCutoff).shiftRight(128).equals(BigInteger.ZERO)) {
          continue;
        }
        if (min[idx] == -1) {
          min[idx] = i;
        }
        if (v.equals(BigInteger.ZERO)) {
          break;
        }
        data.add(v);
        max[idx] = i + 1;
      }
      BigInteger v = BigInteger.TEN.pow(9 * (len + 1))
          .shiftRight(-(k - 16 * idx))
          .mod(BigInteger.TEN.pow(9).shiftLeft(k + 16));
      if (!v.equals(BigInteger.ZERO)) {
        throw new IllegalStateException(idx + " " + len + " " + v);
      }
    }

    System.out.println("static const uint16_t POW10_OFFSET_2[TABLE_SIZE_2] = {");
    System.out.print(" ");
    for (int idx = 0; idx < TABLE_SIZE_2; idx++) {
      System.out.printf(" %4d,", Integer.valueOf(offset[idx]));
      if (idx % 10 == 9) {
        System.out.println();
        System.out.print(" ");
      }
    }
    System.out.printf(" %4d\n", Integer.valueOf(data.size()));
    System.out.println("};");
    System.out.println();

    System.out.println("static const uint8_t MIN_BLOCK_2[TABLE_SIZE_2] = {");
    System.out.print(" ");
    for (int idx = 0; idx < TABLE_SIZE_2; idx++) {
      System.out.printf(" %4d,", Integer.valueOf(min[idx]));
      if (idx % 10 == 9) {
        System.out.println();
        System.out.print(" ");
      }
    }
    System.out.printf(" %4d\n", Integer.valueOf(0));
    System.out.println("};");

//    System.out.println("static const uint8_t MAX_BLOCK_2[TABLE_SIZE_2] = {");
//    System.out.print(" ");
//    for (int idx = 0; idx < TABLE_SIZE_2; idx++) {
//      System.out.printf(" %4d,", Integer.valueOf(max[idx]));
//      if (idx % 10 == 9) {
//        System.out.println();
//        System.out.print(" ");
//      }
//    }
//    System.out.printf(" %4d\n", Integer.valueOf(0));
//    System.out.println("};");

    System.out.println();
    System.out.println("static const uint64_t POW10_SPLIT_2[" + data.size() + "][3] = {");
    for (int i = 0; i < data.size(); i++) {
      BigInteger v = data.get(i);
      System.out.printf("  { ");
      for (int j = 0; j < 3; j++) {
        if (j != 0) {
          System.out.print(", ");
        }
        BigInteger bits = v.shiftRight(64 * j).and(mask);
        System.out.printf("%20su", bits);
      }
      System.out.printf(" },\n");
    }
    System.out.println("};");
  }

  private static int pow10BitsForIndex(int idx) {
    return 16 * idx + POW10_ADDITIONAL_BITS;
  }

  private static int lengthForIndex(int idx) {
    // [log_10(2^i)] = ((16 * i) * 78913L) >> 18 <-- floor
    // +1 for ceil, +16 for mantissa, +8 to round up when dividing by 9
    return ((int) (((16 * idx) * 1292913986L) >> 32) + 1 + 16 + 8) / 9;
  }

  private static int lengthForIndex2(int idx) {
    return (120 * idx) / 64 + 3;
  }
}
