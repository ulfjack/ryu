# Ryu

This project contains a C and a Java implementation of Ryu, an algorithm to
quickly convert floating point numbers to decimal strings. We have tested the
code on Ubuntu 17.10, MacOS High Sierra, and Windows 10.

The Java implementations are RyuFloat and RyuDouble under src/main/java/. The
C implementation is in the ryu/ directory. Both cover 32 and 64-bit floating
point numbers.

All code outside of third_party/ is Copyright Ulf Adams, and published under the
Apache License 2.0.

## Deviations from the (as yet, unpublished) Paper

### Changes to the Algorithm
Given the feedback from the reviewers, we have decided to change the code to
generate output that is closest to the original input, by default. This
results in slightly worse performance compared to the original code. Only the
64-bit C variant still supports the original mode, and this needs to be
enabled with the `LEGACY_MODE` preprocessor symbol.

```
$ bazel run -c opt --copt=-DLEGACY_MODE //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3
64:   23.586    1.542      101.381   98.006
```

We've also added a mode to more closely match Grisu3 output, which can be
enabled by setting the `MATCH_GRISU3_OUTPUT` preprocessor symbol. This only
applies to values that are exactly halfway between two shortest decimal numbers;
the generated strings for all other numbers are unaffected. In this mode, the
benchmark also verfies that the generated strings are identical.
```
$ bazel run -c opt --copt=-DMATCH_GRISU3_OUTPUT //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3
64:   29.806    3.182      103.060   98.717
```

Note that these changes also apply to the computation of the required lookup
table sizes and bit sizes. You can pass the `-legacy` option to the lookup
table and bit size programs to switch to the original mode.

### Errors in the Paper
We had an off-by-one error in `ComputeTableSizes` (Figure 3), which is fixed
in the code shown here. We also changed the output to match the
implementation, which has two zero-indexed tables, one of which is not
strictly necessary, but simplifies the code, so the totals are off by two
compared to the paper.

We were also using an incorrect value in the implementation of
`ComputeRequiredBitSizes` (also Figure 3); this resulted in numbers that are
larger than necessary. In addition, note that our approach is already
unnecessarily conservative; at least for the 32-bit case, we've found through
exhaustive testing that the code still returns correct results for some
smaller bit sizes.

### Jaffer's Implementation
The code given by Jaffer in the original paper, which we used in our paper,
does not come with a license declaration. Instead, we're using code found on
GitHub, which contains a license declaration by Jaffer. This implementation
was fixed to no longer output incorrect values for negative numbers compared
to the original implementation.

## Building, Testing, Running

We use the Bazel build system (https://bazel.build). We recommend using the
latest release, but it should also work with earlier versions. You also need
to install Jdk 8 and a C++ compiler (gcc or clang on Ubuntu, or XCode on
MacOS).


### Building with a Custom Compiler
You can select a custom C++ compiler by setting the CC environment variable
(e.g., on Ubuntu, run `export CC=clang-3.9`).

As of this writing, Bazel does not work with some compilers due to
https://github.com/bazelbuild/bazel/issues/3977 (for example, it does not work
with clang-4.0 or clang-5.0 on Ubuntu 17.10). As a workaround, you can take the
following steps:

 1. set the CC variable to the compiler you want to use
 2. run bazel build once
 3. edit the file pointed at by bazel-ryu/external/local_config_cc/CROSSTOOL
 4. add or replace the cxx_builtin_include_directory directives

For example, for clang-4.0, use these steps:
```
$ export CC=clang-4.0
$ bazel build //ryu
$ sed -i 's|cxx_builtin_include_directory: "/.*"|cxx_builtin_include_directory: "/"|' \
    $(readlink -f bazel-$(basename $PWD)/external/local_config_cc/CROSSTOOL)
```

### Tests
You can run the tests with
```
$ bazel test //ryu/... //src/...
```

### Computing Required Lookup Table Sizes
You can compute the required lookup table sizes with:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ComputeTableSizes --
```

Add `-v` to get slightly more verbose output. Add `-legacy` to match the
original algorithm as described in the paper.

### Computing Required Bit Sizes
You can compute the required bit sizes with:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ComputeRequiredBitSizes --
```

Add the `-128` and `-256` flags to also cover 128- and 256-bit numbers. This
could take a while - 128-bit takes ~20 seconds on my machine while 256-bit takes
a few hours. Add `-v` to get very verbose output. Add `-legacy` to match the
original algorithm as described in the paper.

### Comparing All Possible 32-bit Values Exhaustively
You can check the slow vs. the fast implementation for all 32-bit floating point
numbers using:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ExhaustiveFloatComparison
```

This takes ~60 hours to run to completion on an
Intel(R) Core(TM) i7-4770K with 3.50GHz.

### Comparing All Possible 64-bit Values Exhaustively
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
32:   23.625    1.618       91.647   44.685
64:   29.724    1.650      100.062   83.147

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

### Building without Bazel
You can build and run the C benchmark without using Bazel with the following shell
command:
```
$ gcc -o benchmark -I. -O2 -l stdc++ ryu/*.c ryu/benchmark/benchmark.cc \
    third_party/double-conversion/double-conversion/*.cc \
    third_party/mersenne/*.c
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