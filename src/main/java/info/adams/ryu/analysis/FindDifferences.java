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

import org.yuanheng.cookjson.DoubleUtils;

import info.adams.random.MersenneTwister;
import info.adams.ryu.RoundingMode;
import info.adams.ryu.RyuDouble;

/**
 * A few tests to check that {@link DoubleUtils} isn't completely broken.
 *
 * <p>Note that the implementation does not follow Java {@link Double#toString} semantics, so we
 * can't run the existing double-to-string tests against it.
 */
public class FindDifferences {
  private static final int DOUBLE_MANTISSA_BITS = 52;
  private static final long DOUBLE_MANTISSA_MASK = (1L << DOUBLE_MANTISSA_BITS) - 1;

  private static final int DOUBLE_EXPONENT_BITS = 11;
  private static final int DOUBLE_EXPONENT_MASK = (1 << DOUBLE_EXPONENT_BITS) - 1;
  private static final int DOUBLE_EXPONENT_BIAS = (1 << (DOUBLE_EXPONENT_BITS - 1)) - 1;

  private enum Mode {
    SUMMARY, LATEX, CSV;
  }

  public static void main(String[] args) {
    Mode mode = Mode.SUMMARY;
    for (String s : args) {
      if (s.startsWith("-mode=")) {
        String modeName = s.substring(6);
        switch (modeName) {
          case "summary":
            mode = Mode.SUMMARY;
            break;
          case "latex":
            mode = Mode.LATEX;
            break;
          case "csv":
            mode = Mode.CSV;
            break;
        }
      }
    }
    if (mode == Mode.CSV) {
      System.out.println("compared_to,ryu_output,other_output,exact_lower_bound,exact_value,exact_upper_bound");
    }
    compareWithJaffer(mode);
    compareWithJdk(mode);
  }

  private static void compareWithJaffer(Mode mode) {
    MersenneTwister rand = new MersenneTwister(12345);
    int toolong = 0;
    for (int i = 0; i < 1000000; i++) {
      double value = Double.longBitsToDouble(rand.nextLong());
      if ((Math.abs(value) >= 1E-3) && (Math.abs(value) <= 1E7)) {
        // We ignore differences in the range 1e-3 to 1e7.
        continue;
      }
      String a = DoubleUtils.toString(value);
      String b = RyuDouble.doubleToString(value, RoundingMode.ROUND_EVEN);
      if (a.length() > b.length()) {
        if (toolong < 20 || mode == Mode.CSV) {
          // Only print the first 20.
          printComparison(value, b, a, mode, "Jaffer");
        }
        toolong++;
      }
    }
    if (mode != Mode.CSV) {
      System.out.println("Jaffer: " + toolong + " / " + 100000 + " are longer");
    }
  }

  private static void compareWithJdk(Mode mode) {
    MersenneTwister rand = new MersenneTwister(12345);
    int toolong = 0;
    for (int i = 0; i < 1000000; i++) {
      double value = Double.longBitsToDouble(rand.nextLong());
      String a = Double.toString(value);
      // All of the longer strings from the JDK end in E{16,17,18}.
//      if (a.endsWith("E16") || a.endsWith("E17") || a.endsWith("E18")) {
//        continue;
//      }
      // We're nice and compare with the CONSERVATIVE rounding mode here.
      String b = RyuDouble.doubleToString(value, RoundingMode.CONSERVATIVE);
      if (a.length() > b.length()) {
        if (toolong < 20 || mode == Mode.CSV) {
          // Only print the first 20.
          printComparison(value, b, a, mode, "Jdk");
        }
        toolong++;
      }
    }
    if (mode != Mode.CSV) {
      System.out.println("Jdk: " + toolong + " / " + 100000 + " are longer");
    }
  }

  private static void printComparison(double value, String ryuOutput, String candidate, Mode mode, String area) {
    if (Double.isNaN(value)) return;
    if (value == Float.POSITIVE_INFINITY) return;
    if (value == Float.NEGATIVE_INFINITY) return;
    long bits = Double.doubleToLongBits(value);
    if (bits == 0) return;
    if (bits == 0x80000000) return;
    int ieeeExponent = (int) ((bits >>> DOUBLE_MANTISSA_BITS) & DOUBLE_EXPONENT_MASK);
    long ieeeMantissa = bits & DOUBLE_MANTISSA_MASK;
    int e2;
    long m2;
    if (ieeeExponent == 0) {
      // Denormal number - no implicit leading 1, and the exponent is 1, not 0.
      e2 = 1 - DOUBLE_EXPONENT_BIAS - DOUBLE_MANTISSA_BITS;
      m2 = ieeeMantissa;
    } else {
      // Add implicit leading 1.
      e2 = ieeeExponent - DOUBLE_EXPONENT_BIAS - DOUBLE_MANTISSA_BITS;
      m2 = ieeeMantissa | (1L << DOUBLE_MANTISSA_BITS);
    }

    long mv = 4 * m2;
    long mp = 4 * m2 + 2;
    long mm = 4 * m2 - (((m2 != (1L << DOUBLE_MANTISSA_BITS)) || (ieeeExponent <= 1)) ? 2 : 1);
    e2 -= 2;

    BigInteger vr, vp, vm;
    int e10;
    if (e2 >= 0) {
      vr = BigInteger.valueOf(mv).shiftLeft(e2);
      vp = BigInteger.valueOf(mp).shiftLeft(e2);
      vm = BigInteger.valueOf(mm).shiftLeft(e2);
      e10 = 0;
    } else {
      BigInteger factor = BigInteger.valueOf(5).pow(-e2);
      vr = BigInteger.valueOf(mv).multiply(factor);
      vp = BigInteger.valueOf(mp).multiply(factor);
      vm = BigInteger.valueOf(mm).multiply(factor);
      e10 = e2;
    }
    e10 += vp.toString().length() - 1;
    String vms = vm.toString().replaceAll("^(.)", "$1.");
    String vrs = vr.toString().replaceAll("^(.)", "$1.");
    String vps = vp.toString().replaceAll("^(.)", "$1.");

    if (mode == Mode.LATEX) {
      int maxLen = 24;
      if (vps.length() > maxLen) {
        vms = vms.substring(0, Math.min(maxLen, vms.length())) + "...";
        vrs = vrs.substring(0, Math.min(maxLen, vrs.length())) + "...";
        vps = vps.substring(0, Math.min(maxLen, vps.length())) + "...";
      }
      System.out.printf("\\texttt{0x%s} & ", Long.toHexString(bits));
      System.out.printf("$\\texttt{%s} * 10^{%s}$ & ", vms, Integer.valueOf(e10));
      System.out.printf("$\\texttt{%s}$ (our) \\\\\n", SlowConversion.doubleToString(value));
  
      System.out.printf(" & ");
      System.out.printf("$\\texttt{%s} * 10^{%s}$ & ", vrs, Integer.valueOf(e10));
      System.out.printf("$\\texttt{%s}$ (%s) \\\\\n", candidate, "OpenJDK");
  
      System.out.printf(" & ");
      System.out.printf("$\\texttt{%s} * 10^{%s}$ ", vps, Integer.valueOf(e10));
      System.out.printf("& \\\\ \\hline\n");
    } else if (mode == Mode.CSV) {
      System.out.println(area + "," + ryuOutput + "," + candidate + "," + vms + "," + vrs + "," + vps);
    }
  }
}