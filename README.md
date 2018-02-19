# Ryu

This project contains a C and a Java implementation of Ryu, an algorithm to
quickly convert floating point numbers to decimal strings.

The Java implementations are RyuFloat and RyuDouble under src/main/java/. The
C implementation is in the ryu/ directory. Both cover 32 and 64-bit floating
point numbers.

All code outside of third_party/ is Copyright Ulf Adams, and published under the
Apache License 2.0.

## Building, Testing, Running

We use the Bazel build system (https://bazel.build). We recommend using the
latest release, but it should also work with earlier versions.

### Tests
You can run the tests with
```
$ bazel test //ryu/... //src/...
```

### Computing Required Bit Sizes
You can compute the required bit sizes with:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ComputeRequiredBitSizes --
```

Add the `-128` and `-256` flags to also cover 128- and 256-bit numbers. This
could take a while - 128-bit takes ~20 seconds on my machine while 256-bit takes
a few hours. Add `-v` to get very verbose output.

### Comparing All Possible 32-bit Values Exhaustively
You can check the slow vs. the fast implementation for all 32-bit floating point
numbers using:
```
$ bazel run //src/main/java/info/adams/ryu/analysis:ExhaustiveFloatComparison
```

This takes ~60 hours to run to completion.

### Benchmarks
We provide both C and Java benchmark programs.

Enable optimization by adding "-c opt" on the command line, and select a custom
C/C++ compiler by setting the CC environment variable before running bazel,
e.g.:
```
$ CC=clang-3.9 bazel run -c opt //ryu/benchmark --
    Average & Stddev Ryu  Average & Stddev Grisu3  (----------)
32:   23.561    1.593       90.875   44.514           11443598
64:   33.805    1.713      100.227   97.514           13403191

$ bazel run //src/main/java/info/adams/ryu/benchmark --
    Average & Stddev Ryu  Average & Stddev JDK  (----------)
32:   55.879   11.802       255.617  172.629       24455000
64:   88.724    7.411      1069.786  295.320       43958000
```

As of this writing, Bazel does not work with clang-4.0 or clang-5.0 due to
https://github.com/bazelbuild/bazel/issues/3977.

Additional parameters can be passed to the benchmark after the `--` parameter:
```
  -32           only run the 32-bit benchmark
  -64           only run the 64-bit benchmark
  -samples=n    run n pseudo-randomly selected numbers
  -iterations=n run each number n times
  -v            generate verbose output in CSV format
```

## Deviations from the (as yet, unpublished) Paper

Given the feedback from the reviewers, we have decided to change the code to
generate output that is closest to the original input, by default. Only the
64-bit C variant still supports the original mode as described in the paper,
and this needs to be enabled with the `LEGACY_MODE` preprocessor symbol.

```
$ CC=clang-3.9 bazel run -c opt --copt=-DLEGACY_MODE //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3  (--------)
64:   23.586    1.542      101.381   98.006         12496731
```

We've also added a mode to more closely match Grisu3 output, which can be
enabled by setting the `MATCH_GRISU3_OUTPUT` preprocessor symbol. This only
applies to values that are exactly halfway between two shortest decimal numbers;
the generated strings for all other numbers are unaffected. In this mode, the
benchmark also verfies that the generated strings are identical.
```
$ CC=clang-3.9 bazel run -c opt --copt=-DMATCH_GRISU3_OUTPUT //ryu/benchmark -- -64
    Average & Stddev Ryu  Average & Stddev Grisu3  (--------)
64:   29.806    3.182      103.060   98.717         13286634
```
