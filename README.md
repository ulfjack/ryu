# Ryu [![Build Status](https://travis-ci.org/ulfjack/ryu.svg?branch=master)](https://travis-ci.org/ulfjack/ryu)

This project contains C and Java implementation of Ryu, as well as a C
implementation of Ryu Printf. Ryu converts a floating point number to its
shortest decimal representation, whereas Ryu Printf converts a floating point
number according to the printf ```%f``` or ```%e``` format. At the time of this
writing, these are the fastest known float-to-string conversion algorithms. We
have tested the code on Ubuntu 19.04, MacOS Mojave, and Windows 10.

All code outside of third_party/ is copyrighted by Ulf Adams and contributors,
and may be used freely in accordance with the Apache 2.0 license.
Alternatively, the files in the ryu/ directory may be used freely in accordance
with the Boost 1.0 license.

All contributions are required to maintain these licenses.


## Ryu
Ryu generates the shortest decimal representation of a floating point number
that maintains round-trip safety. That is, a correct parser can recover the
exact original number. For example, consider the binary 64-bit floating point
number ```00111110100110011001100110011010```. The stored value is exactly
```0.300000011920928955078125```. However, this floating point number is also
the closest number to the decimal number ```0.3```, so that is what Ryu
outputs.

This problem of generating the shortest possible representation was originally
posed by White and Steele [1], for which they described an algorithm called
"Dragon". It was subsequently improved upon with algorithms that also had
dragon-themed names. I followed in the same vein using the japanese word for
dragon, Ryu. In general, all these algorithms should produce identical output
given identical input.

The C implementation of Ryu is in the ryu/ directory. The Java implementations
are RyuFloat and RyuDouble under src/main/java/. Both cover 32 and 64-bit
floating point numbers.

In addition, there is an experimental C implementation that can handle inputs
of any size up to 128-bit, albeit with lower performance than the highly
optimized 32-bit and 64-bit implementations. Furthermore, there is an
experimental low-level C API that returns the decimal floating-point
representation as a struct, allowing clients to implement their own formatting.
These are still subject to change.

*Note*: The Java implementation differs from the output of ```Double.toString```
[2] in some cases: sometimes the output is shorter (which is arguably more
accurate) and sometimes the output may differ in the precise digits output
(e.g., see https://github.com/ulfjack/ryu/issues/83).

*Note*: While the Java specification requires outputting at least 2 digits,
other specifications, such as for JavaScript, always require the shortest output.
We may change the Java implementation in the future to support both.

My PLDI'18 paper includes a complete correctness proof of the algorithm:
https://dl.acm.org/citation.cfm?id=3192369, available under the creative commons
CC-BY-SA license.

Other implementations of Ryu:

| Language         | Author             | Link                                          |
|------------------|--------------------|-----------------------------------------------|
| Scala            | Andriy Plokhotnyuk | https://github.com/plokhotnyuk/jsoniter-scala |
| Rust             | David Tolnay       | https://github.com/dtolnay/ryu                |
| Julia            | Jacob Quinn        | https://github.com/quinnj/Ryu.jl              |
| Factor           | Alexander Iljin    | https://github.com/AlexIljin/ryu              |
| Go               | Caleb Spare        | https://github.com/cespare/ryu                |

[1] "How to Print Floating-Point Numbers Accurately", PLDI '90, https://dl.acm.org/citation.cfm?id=93559
[2] https://docs.oracle.com/javase/10/docs/api/java/lang/Double.html#toString(double)

### Comparison with Other Implementations

#### Grisu3

Ryu's output should exactly match Grisu3's output. Our benchmark verifies that
the generated numbers are identical.
```
$ bazel run -c opt //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3
64:   29.806    3.182      103.060   98.717
```

#### Jaffer's Implementation
The code given by Jaffer in the original paper does not come with a license
declaration. Instead, we're using code found on GitHub, which contains a
license declaration by Jaffer. Compared to the original code, this
implementation no longer outputs incorrect values for negative numbers.

#### Differences between Ryu and Jaffer / Jdk implementations
We provide a binary to find differences between Ryu and the Jaffer / Jdk
implementations:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:FindDifferences --
```

Add the `-mode=csv` option to get all the discovered differences as a CSV. Use
`-mode=latex` instead to get a latex snippet of the first 20. Use
`-mode=summary` to only print the number of discovered differences (this is the
default mode).

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

This takes approximately forever, so you will need to interrupt the program.



## Ryu Printf
Since Ryu generates the shortest decimal representation, it is not immediately
suitable for use in languages that have printf-like facilities. In most
implementations, printf provides three floating-point specific formatters,
```%f```, ```%e```, and ```%g```:

 - The ```%f``` format prints the full decimal part of the given floating point
   number, and then appends as many digits of the fractional part as specified
   using the precision parameter.

 - The ```%e``` format prints the decimal number in scientific notation with as
   many digits after the initial digit as specified using the precision
   parameter.

 - The ```%g``` format prints either ```%f``` or ```%e``` format, whichever is
   shorter.

Ryu Printf implements %f and %e formatting in a way that should be drop-in
compatible with most implementations of printf, although it currently does not
implement any formatting flags other than precision.

According to our benchmarks, Ryu Printf compares favorably with the following
implementations of printf for precision parameters 1, 10, 100, and 1000:

| OS                   | Libc                        | Ryu Printf is faster by |
|----------------------|-----------------------------|-------------------------|
| Ubuntu 18.04         | libc6 2.27-3ubuntu1         | 15x                     |
| Ubuntu 18.04         | musl 1.1.19-1               | 4x                      |
| Windows 10 Home 1803 | MSVC 19.14.26429.4          | 9x                      |
| Windows 10 Home 1803 | msys-runtime-devel 2.10.0-2 | between 8x and 20x      |
| macOS Mojave 10.14   | Apple Libc                  | 24x                     |

In addition, Ryu Printf has a more predictable performance profile. In theory,
an implementation that performs particularly badly for some subset of numbers
could be exploited as a denial-of-service attack vector.


## Building, Testing, Running

We use the Bazel build system (https://bazel.build) 0.14 or later, although we
recommend using the latest release. You also need to install Jdk 8 (or later)
to build and run the Java code, and/or a C/C++ compiler (gcc or clang on Ubuntu,
XCode on MacOS, or MSVC on Windows) to build the C/C++ code.

### Building with a Custom Compiler
You can select a custom C++ compiler by setting the CC environment variable
(e.g., on Ubuntu, run `export CC=clang-3.9`).

For example, use these steps to build with clang-4.0 on Ubuntu:
```
$ export CC=clang-4.0
$ bazel build //ryu
```

### Tests
You can run both C and Java tests with
```
$ bazel test //ryu/... //src/...
```

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
