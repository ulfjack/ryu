# Ryu & Ryu Printf [![Build Status](https://travis-ci.org/ulfjack/ryu.svg?branch=master)](https://travis-ci.org/ulfjack/ryu)

This project contains routines to convert IEEE-754 floating-point numbers to
decimal strings using shortest, fixed `%f`, and scientific `%e`
formatting. The primary implementation is in C, and there is a port of the
shortest conversion to Java. All algorithms have been published in
peer-reviewed publications. At the time of this writing, these are the fastest
known float-to-string conversion algorithms. The fixed, and scientific
conversion routines are several times faster than the usual implementations
of sprintf (we compared against glibc, Apple's libc, MSVC, and others).

Generating scientific and fixed output format for 16 and 32 bit IEEE floating
point numbers can be implemented by converting to 64 bit, and then using the
64 bit routines. Note that there is no 128 bit implementation at this time.

When converting to shortest, DO NOT CAST; shortest conversion is based on the
precision of the source type, and casting to a different type will not return
the expected output. There are highly optimized 32 and 64 bit implementations
as well as a generic 128 bit implementation that can handle any IEEE format
up to 128 bits.

These are the supported conversion modes for the C implementation:

| IEEE Type            | Supported Output Formats         |
| -------------------- | -------------------------------- |
| 16 Bit (half)        | Shortest (via ryu_generic_128.h) |
| 32 Bit (float)       | Shortest                         |
| 64 Bit (double)      | Shortest, Scientific, Fixed      |
| 80 Bit (long double) | Shortest (via ryu_generic_128.h) |
| 128 Bit (__float128) | Shortest (via ryu_generic_128.h) |

The code is continuously tested on Ubuntu 18.04, MacOS High Sierra, and Windows
Server version 1803.

All code outside of third_party/ is copyrighted by Ulf Adams and contributors,
and may be used freely in accordance with the Apache 2.0 license.
Alternatively, the files in the ryu/ directory may be used freely in accordance
with the Boost 1.0 license.

All contributions are required to maintain these licenses.

## Ryu
Ryu generates the shortest decimal representation of a floating point number
that maintains round-trip safety. That is, a correct parser can recover the
exact original number. For example, consider the binary 32-bit floating point
number `00111110100110011001100110011010`. The stored value is exactly
`0.300000011920928955078125`. However, this floating point number is also
the closest number to the decimal number `0.3`, so that is what Ryu
outputs.

This problem of generating the shortest possible representation was originally
posed by White and Steele [[1]], for which they described an algorithm called
"Dragon". It was subsequently improved upon with algorithms that also had
dragon-themed names. I followed in the same vein using the japanese word for
dragon, Ryu. In general, all these algorithms should produce identical output
given identical input, and this is checked when running the benchmark program.

The C implementation of Ryu is in the ryu/ directory. The Java implementations
are RyuFloat and RyuDouble under src/main/java/. Both cover 32 and 64-bit
floating point numbers.

In addition, there is an experimental C implementation that can handle inputs
of any size up to 128-bit, albeit with lower performance than the highly
optimized 32-bit and 64-bit implementations. Furthermore, there is an
experimental low-level C API that returns the decimal floating-point
representation as a struct, allowing clients to implement their own formatting.
These are still subject to change.

*Note*: The Java implementation differs from the output of `Double.toString`
[[2]] in some cases: sometimes the output is shorter (which is arguably more
accurate) and sometimes the output may differ in the precise digits output
(e.g., see https://github.com/ulfjack/ryu/issues/83).

*Note*: While the Java specification requires outputting at least 2 digits,
other specifications, such as for JavaScript, always require the shortest output.
We may change the Java implementation in the future to support both.

My PLDI'18 paper includes a complete correctness proof of the algorithm:
https://dl.acm.org/citation.cfm?doid=3296979.3192369

Other implementations of Ryu:

| Language         | Author             | Link                                          |
|------------------|--------------------|-----------------------------------------------|
| Scala            | Andriy Plokhotnyuk | [https://github.com/plokhotnyuk/jsoniter-scala][3] |
| Rust             | David Tolnay       | https://github.com/dtolnay/ryu                |
| Julia            | Jacob Quinn        | https://github.com/JuliaLang/julia/tree/master/base/ryu |
| Factor           | Alexander Iljin    | https://github.com/AlexIljin/ryu              |
| Go               | Caleb Spare        | https://github.com/cespare/ryu                |
| C#               | Dogwei             | https://github.com/Dogwei/RyuCsharp           |
| C#               | Shad Storhaug      | https://github.com/NightOwl888/J2N            |
| D                | Ilya Yaroshenko    | [https://github.com/libmir/mir-algorithm][5]  |
| Scala            | Denys Shabalin     | [https://github.com/scala-native/scala-native][4] |
| Erlang/BEAM      | Thomas Depierre    | https://github.com/erlang/otp/tree/master/erts/emulator/ryu |
| Zig              | Marc Tiehuis       | https://github.com/tiehuis/zig-ryu            |
| Haskell          | [Lawrence Wu](https://github.com/la-wu) | [https://github.com/haskell/bytestring][6] |

[1]: https://dl.acm.org/citation.cfm?id=93559

[2]: https://docs.oracle.com/javase/10/docs/api/java/lang/Double.html#toString(double)

[3]: https://github.com/plokhotnyuk/jsoniter-scala/blob/6e6bb9d7bed6de341ce0b781b403eb671d008468/jsoniter-scala-core/jvm/src/main/scala/com/github/plokhotnyuk/jsoniter_scala/core/JsonWriter.scala#L1777-L2262

[4]: https://github.com/scala-native/scala-native/tree/master/nativelib/src/main/scala/scala/scalanative/runtime/ieee754tostring/ryu

[5]: https://github.com/libmir/mir-algorithm/tree/master/source/mir/bignum/internal/ryu

[6]: https://github.com/haskell/bytestring/pull/365

## Ryu Printf
Since Ryu generates the shortest decimal representation, it is not immediately
suitable for use in languages that have printf-like facilities. In most
implementations, printf provides three floating-point specific formatters,
`%f`, `%e`, and `%g`:

 - The `%f` format prints the full decimal part of the given floating point
   number, and then appends as many digits of the fractional part as specified
   using the precision parameter.

 - The `%e` format prints the decimal number in scientific notation with as
   many digits after the initial digit as specified using the precision
   parameter.

 - The `%g` format prints either `%f` or `%e` format, whichever is
   shorter.

Ryu Printf implements %f and %e formatting in a way that should be drop-in
compatible with most implementations of printf, although it currently does not
implement any formatting flags other than precision. The benchmark program
verifies that the output matches exactly, and outputs a warning if not. Any
unexpected output from the benchmark indicates a difference in output.

*Note* that old versions of MSVC ship with a printf implementation that has a
confirmed bug: it does not always round the last digit correctly.

*Note* that msys cuts off the output after ~17 digits, and therefore generally
differs from Ryu Printf output for precision values larger than 17.

*Note* that the output for NaN values can differ between implementations; we use
ifdefs in an attempt to match platform output.

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

My OOPSLA'2019 paper provides a correctness proof:
https://dl.acm.org/citation.cfm?doid=3366395.3360595



## Building, Testing, Running

We use the Bazel build system (https://bazel.build) 0.14 or later, although we
recommend using the latest release. You also need to install Jdk 8 (or later)
to build and run the Java code, and/or a C/C++ compiler (gcc or clang on Ubuntu,
XCode on MacOS, or MSVC on Windows) to build the C/C++ code.

To build Ryu, run
```
$ bazel build //ryu
```

To build Ryu Printf, run
```
$ bazel build //ryu:ryu_printf
```

### Big-Endian Architectures
The C implementations should work on big-endian architectures provided that the
floating point type and the corresponding integer type use the same endianness.

There are no concerns around endianness for the Java implementation.

### Building with a Custom Compiler
You can select a custom C++ compiler by setting the CC environment variable,
e.g., use these steps to build with clang-4.0 on Ubuntu:
```
$ export CC=clang-4.0
$ bazel build //ryu
```

Building Ryu Printf against musl and msys requires installing the corresponding
packages. We only tested against the musl Debian package that installs a gcc
wrapper and is enabled by setting `CC`. However, building against msys
requires manually adjusting Bazel's compiler configuration files.

### Tests
You can run both C and Java tests with
```
$ bazel test //ryu/... //src/...
```



## Ryu: Additional Notes

### Jaffer
The code given by Jaffer in the original paper does not come with a license
declaration. Instead, we're using code found on GitHub [3], which contains a
license declaration by Jaffer. Compared to the original code, this
implementation no longer outputs incorrect values for negative numbers.

We provide a binary to find differences between Ryu and the Jaffer / Jdk
implementations:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:FindDifferences --
```

Add the `-mode=csv` option to get all the discovered differences as a CSV. Use
`-mode=latex` instead to get a latex snippet of the first 20. Use
`-mode=summary` to only print the number of discovered differences (this is the
default mode).

[3]: https://github.com/coconut2015/cookjson/blob/master/cookjson-core/src/main/java/org/yuanheng/cookjson/DoubleUtils.java

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



## Benchmarks

### Ryu
We provide both C and Java benchmark programs.

Enable optimization by adding "-c opt" on the command line:
```
$ bazel run -c opt //ryu/benchmark:ryu_benchmark --
    Average & Stddev Ryu  Average & Stddev Grisu3
32:   22.515    1.578       90.981   41.455
64:   27.545    1.677       98.981   80.797
```

For the Java benchmark, run:
```
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
  -ryu          run Ryu only, no comparison
  -v            generate verbose output in CSV format
```

If you have gnuplot installed, you can generate plots from the benchmark data
with:
```
$ bazel build -c opt --jobs=1 //scripts:shortest-{c,java}-{float,double}.pdf
```

The resulting files are `bazel-genfiles/scripts/shortest-{c,java}-{float,double}.pdf`.

### Ryu Printf
We provide a C++ benchmark program that runs against the implementation of
`snprintf` bundled with the selected C++ compiler. You need to enable
optimization using "-c opt" on the command line:
```
$ bazel run -c opt //ryu/benchmark:ryu_printf_benchmark --
    Average & Stddev Ryu  Average & Stddev snprintf
%f:  116.359  130.992     3983.251 5331.505
%e:   40.853   10.872      210.648   36.779
```

Additional parameters can be passed to the benchmark after the `--` parameter:
```
  -f            only run the %f benchmark
  -e            only run the %e benchmark
  -precision=n  run with precision n (default is 6)
  -samples=n    run n pseudo-randomly selected numbers
  -iterations=n run each number n times
  -ryu          run Ryu Printf only, no comparison
  -v            generate verbose output in CSV format
```

See above for selecting a different compiler. Note that msys C++ compilation
does not work out of the box.

We also provide a simplified C benchmark for platforms that do not support C++
compilation, but *note* that pure C compilation is not natively supported by
Bazel:
```
$ bazel run -c opt //ryu/benchmark:ryu_printf_benchmark_c --
```

If you have gnuplot installed, you can generate plots from the benchmark data
with:
```
$ bazel build -c opt --jobs=1 //scripts:{f,e}-c-double-{1,10,100,1000}.pdf
```

The resulting files are `bazel-genfiles/scripts/{f,e}-c-double-{1,10,100,1000}.pdf`.
