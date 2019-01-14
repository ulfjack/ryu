# Ryu [![Build Status](https://travis-ci.org/ulfjack/ryu.svg?branch=master)](https://travis-ci.org/ulfjack/ryu)

This project contains a C and a Java implementation of Ryu, an algorithm to
quickly convert floating point numbers to decimal strings. We have tested the
code on Ubuntu 17.10, MacOS High Sierra, and Windows 10.

The Java implementations are RyuFloat and RyuDouble under src/main/java/. The
C implementation is in the ryu/ directory. Both cover 32 and 64-bit floating
point numbers.

*Note*: The Java implementation follows the Java specification for
Double.toString [1], which requires outputting at least two digits. Other
specifications, such as for JavaScript, require the shortest output. We may
change the Java implementation in the future to support both.

There is an experimental C low-level API and 128-bit implementation in ryu/.
These are still subject to change.

All code outside of third_party/ is Copyright Ulf Adams, and may be used in
accordance with the Apache 2.0 license. Alternatively, the files in the ryu/
directory may be used in accordance with the Boost 1.0 license.

My PLDI'18 paper includes a complete correctness proof of the algorithm:
https://dl.acm.org/citation.cfm?id=3192369, available under the creative commons
CC-BY-SA license.

Other implementations:

| Language         | Author             | Link                                          |
|------------------|--------------------|-----------------------------------------------|
| Scala            | Andriy Plokhotnyuk | https://github.com/plokhotnyuk/jsoniter-scala |
| Rust             | David Tolnay       | https://github.com/dtolnay/ryu                |
| Julia            | Jacob Quinn        | https://github.com/quinnj/Ryu.jl              |
| Factor           | Alexander Iljin    | https://github.com/AlexIljin/ryu              |
| Go               | Caleb Spare        | https://github.com/cespare/ryu                |

[1] https://docs.oracle.com/javase/10/docs/api/java/lang/Double.html#toString(double)

## Building, Testing, Running

We use the Bazel build system (https://bazel.build). We recommend using the
latest release, but it should also work with earlier versions. You also need
to install Jdk 8 and a C/C++ compiler (gcc or clang on Ubuntu, XCode on
MacOS, or MSVC on Windows).

### Building with a Custom Compiler
You can select a custom C++ compiler by setting the CC environment variable
(e.g., on Ubuntu, run `export CC=clang-3.9`).

For example, use these steps to build with clang-4.0:
```
$ export CC=clang-4.0
$ bazel build //ryu
```
Note that older Bazel versions (< 0.14) did not work with all compilers
(https://github.com/bazelbuild/bazel/issues/3977).

### Tests
You can run both C and Java tests with
```
$ bazel test //ryu/... //src/...
```

### Big-Endian Architectures
The C implementation of Ryu should work on big-endian architectures provided
that the floating point type and the corresponding integer type use the same
endianness.

There are no concerns around endianness for the Java implementation.

### Computing Required Lookup Table Sizes
You can compute the required lookup table sizes with:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ComputeTableSizes --
```

Add `-v` to get slightly more verbose output.

### Computing Required Bit Sizes
You can compute the required bit sizes with:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ComputeRequiredBitSizes --
```

Add the `-128` and `-256` flags to also cover 128- and 256-bit numbers. This
could take a while - 128-bit takes ~20 seconds on my machine while 256-bit takes
a few hours. Add `-v` to get very verbose output.

### Java: Comparing All Possible 32-bit Values Exhaustively
You can check the slow vs. the fast implementation for all 32-bit floating point
numbers using:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ExhaustiveFloatComparison
```

This takes ~60 hours to run to completion on an
Intel(R) Core(TM) i7-4770K with 3.50GHz.

### Java: Comparing All Possible 64-bit Values Exhaustively
You can check the slow vs. the fast implementation for all 64-bit floating point
numbers using:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ExtensiveDoubleComparison
```

However, this takes approximately forever, so you will need to interrupt the
program.

### Benchmarks
We provide both C and Java benchmark programs.

Enable optimization by adding "-c opt" on the command line:
```
$ bazel run -c opt //ryu/benchmark --
    Average & Stddev Ryu  Average & Stddev Grisu3
32:   22.515    1.578       90.981   41.455
64:   27.545    1.677       98.981   80.797

$ bazel run //src/main/java/info/adams/ryu/benchmark --
    Average & Stddev Ryu  Average & Stddev Jdk  Average & Stddev Jaffer
32:   56.680    9.127       254.903  170.099
64:   89.751   13.442      1085.596  302.371     1089.535  309.245
```

Additional parameters can be passed to the benchmark after the `--` parameter:
```
  -32           only run the 32-bit benchmark
  -64           only run the 64-bit benchmark
  -samples=n    run n pseudo-randomly selected numbers
  -iterations=n run each number n times
  -v            generate verbose output in CSV format
```

If you have gnuplot installed, you can generate plots from the benchmark data
with:
```
$ bazel build --jobs=1 //scripts:{c,java}-{float,double}.pdf
```

The resulting files are `bazel-genfiles/scripts/{c,java}-{float,double}.pdf`.

### Building without Bazel on Linux / MacOS
You can build and run the C benchmark without using Bazel with the following shell
command:
```
$ gcc -o benchmark -I. -O2 -l m -l stdc++ ryu/*.c ryu/benchmark/benchmark.cc \
    third_party/double-conversion/double-conversion/*.cc
$ ./benchmark
```

You can build and run the Java benchmark with the following shell command:
```
$ mkdir out
$ javac -d out \
    -sourcepath src/main/java/:third_party/mersenne_java/java/:third_party/jaffer/java/ \
    src/main/java/info/adams/ryu/benchmark/BenchmarkMain.java
$ java -cp out info.adams.ryu.benchmark.BenchmarkMain
```

## Comparison with Other Implementations

### Grisu3

Ryu's output should exactly match Grisu3's output. Our benchmark verifies that
the generated numbers are identical.
```
$ bazel run -c opt //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3
64:   29.806    3.182      103.060   98.717
```

### Jaffer's Implementation
The code given by Jaffer in the original paper does not come with a license
declaration. Instead, we're using code found on GitHub, which contains a
license declaration by Jaffer. Compared to the original code, this
implementation no longer outputs incorrect values for negative numbers.

### Differences between Ryu and Jaffer / Jdk implementations
We provide a binary to find differences between Ryu and the Jaffer / Jdk
implementations:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:FindDifferences --
```

Add the `-mode=csv` option to get all the discovered differences as a CSV. Use
`-mode=latex` instead to get a latex snippet of the first 20. Use
`-mode=summary` to only print the number of discovered differences (this is the
default mode).
