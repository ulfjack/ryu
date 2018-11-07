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

package info.adams.ryu.benchmark;

import org.yuanheng.cookjson.DoubleUtils;

import info.adams.random.MersenneTwister;
import info.adams.ryu.RyuDouble;
import info.adams.ryu.RyuFloat;

public class BenchmarkMain {
  public static void main(String[] args) {
    boolean run32 = true;
    boolean run64 = true;
    int samples = 1000;
    int iterations = 1000;
    boolean verbose = false;
    for (String s : args) {
      if (s.equals("-32")) {
        run32 = true;
        run64 = false;
      } else if (s.equals("-64")) {
        run32 = false;
        run64 = true;
      } else if (s.equals("-v")) {
        verbose = true;
      } else if (s.startsWith("-samples=")) {
        samples = Integer.parseInt(s.substring(9));
      } else if (s.startsWith("-iterations=")) {
        iterations = Integer.parseInt(s.substring(12));
      }
    }

    if (verbose) {
      System.out.println("output_ryu,float_bits_as_int,ryu_time_in_ns,jdk_time_in_ns,jaffer_time_in_ns");
    } else {
      System.out.printf("    Average & Stddev Ryu  Average & Stddev Jdk  Average & Stddev Jaffer\n");
    }
    int throwaway = 0;
    if (run32) {
      throwaway += bench32(samples, iterations, verbose);
    }
    if (run64) {
      throwaway += bench64(samples, iterations, verbose);
    }
    if (args.length == 1000) {
      // Prevent the compiler from optimizing the code away. Technically, the
      // JIT could see that args.length != 1000, but it seems unlikely.
      System.err.println(throwaway);
    }
  }

  private static int bench32(int samples, int iterations, boolean verbose) {
    MersenneTwister twister = new MersenneTwister(12345);
    // Warm up the JIT.
    MeanAndVariance warmUp = new MeanAndVariance();
    for (int i = 0; i < 1000; i++) {
      System.gc();
      System.gc();
      int r = twister.nextInt();
      float f = Float.intBitsToFloat(r);
      long start = System.nanoTime();
      for (int j = 0; j < 100; j++) {
        RyuFloat.floatToString(f).length();
      }
      for (int j = 0; j < 100; j++) {
        Float.toString(f).length();
      }
      long stop = System.nanoTime();
      warmUp.update((stop - start) / 100.0);
    }

    MeanAndVariance mv1 = new MeanAndVariance();
    MeanAndVariance mv2 = new MeanAndVariance();
    twister.setSeed(12345);
    // We track some value computed from the result of the conversion to make sure that the
    // compiler does not eliminate the conversion due to the result being otherwise unused.
    int throwaway = 0;
    for (int i = 0; i < samples; i++) {
      System.gc();
      System.gc();
      int r = twister.nextInt();
      float f = Float.intBitsToFloat(r);
      long start = System.nanoTime();
      for (int j = 0; j < iterations; j++) {
        throwaway += RyuFloat.floatToString(f).length();
      }
      long stop = System.nanoTime();
      double delta1 = (stop - start) / (double) iterations;
      mv1.update(delta1);

      System.gc();
      System.gc();
      start = System.nanoTime();
      for (int j = 0; j < iterations; j++) {
        throwaway += Float.toString(f).length();
      }
      stop = System.nanoTime();
      double delta2 = (stop - start) / (double) iterations;
      mv2.update(delta2);
      if (verbose) {
        System.out.printf(
            "%s,%s,%s,%s,\n",
            RyuFloat.floatToString(f),
            Integer.toUnsignedString(r),
            Double.valueOf(delta1),
            Double.valueOf(delta2));
      }
    }
    if (!verbose) {
      System.out.printf("32: %8.3f %8.3f      %8.3f %8.3f\n",
          Double.valueOf(mv1.mean()), Double.valueOf(mv1.stddev()),
          Double.valueOf(mv2.mean()), Double.valueOf(mv2.stddev()));
    }
    return throwaway;
  }

  private static int bench64(int samples, int iterations, boolean verbose) {
    MersenneTwister twister = new MersenneTwister(12345);
    // Warm up the JIT.
    MeanAndVariance warmUp = new MeanAndVariance();
    for (int i = 0; i < 1000; i++) {
      System.gc();
      System.gc();
      long r = twister.nextLong();
      double f = Double.longBitsToDouble(r);
      long start = System.nanoTime();
      for (int j = 0; j < 100; j++) {
        RyuDouble.doubleToString(f).length();
      }
      for (int j = 0; j < 100; j++) {
        Double.toString(f).length();
      }
      for (int j = 0; j < 100; j++) {
        DoubleUtils.toString(f).length();
      }
      long stop = System.nanoTime();
      warmUp.update((stop - start) / 100.0);
    }

    MeanAndVariance mv1 = new MeanAndVariance();
    MeanAndVariance mv2 = new MeanAndVariance();
    MeanAndVariance mv3 = new MeanAndVariance();
    twister.setSeed(12345);
    // We track some value computed from the result of the conversion to make sure that the
    // compiler does not eliminate the conversion due to the result being otherwise unused.
    int throwaway = 0;
    for (int i = 0; i < samples; i++) {
      System.gc();
      System.gc();
      long r = twister.nextLong();
      double f = Double.longBitsToDouble(r);
      long start = System.nanoTime();
      for (int j = 0; j < iterations; j++) {
        throwaway += RyuDouble.doubleToString(f).length();
      }
      long stop = System.nanoTime();
      double delta1 = (stop - start) / (double) iterations;
      mv1.update(delta1);

      System.gc();
      System.gc();
      start = System.nanoTime();
      for (int j = 0; j < iterations; j++) {
        throwaway += Double.toString(f).length();
      }
      stop = System.nanoTime();
      double delta2 = (stop - start) / (double) iterations;
      mv2.update(delta2);

      System.gc();
      System.gc();
      start = System.nanoTime();
      for (int j = 0; j < iterations; j++) {
        throwaway += DoubleUtils.toString(f).length();
      }
      stop = System.nanoTime();
      double delta3 = (stop - start) / (double) iterations;
      mv3.update(delta3);
      if (verbose) {
        System.out.printf(
            "%s,%s,%s,%s,%s\n",
            RyuDouble.doubleToString(f),
            Long.toUnsignedString(r),
            Double.valueOf(delta1),
            Double.valueOf(delta2),
            Double.valueOf(delta3));
      }
    }
    if (!verbose) {
      System.out.printf("64: %8.3f %8.3f      %8.3f %8.3f     %8.3f %8.3f\n",
          Double.valueOf(mv1.mean()), Double.valueOf(mv1.stddev()),
          Double.valueOf(mv2.mean()), Double.valueOf(mv2.stddev()),
          Double.valueOf(mv3.mean()), Double.valueOf(mv3.stddev()));
    }
    return throwaway;
  }
}
