#!/bin/bash
for i in 1 10 100 1000; do
  echo $i
  CC=clang-6.0 bazel run -c opt //ryu/benchmark:benchmark_fixed -- -samples=1000 -v -precision=$i > glibc-fixed-$i.csv
  CC=clang-6.0 bazel run -c opt //ryu/benchmark:benchmark_fixed -- -samples=1000 -v -precision=$i -exp > glibc-exp-$i.csv
done

#for i in 1 10 100 1000; do
#  echo $i
#  CC=musl-gcc bazel run -c opt //ryu/benchmark:benchmark_fixed -- -samples=1000 -v -precision=$i > musl-fixed-$i.csv
#  CC=musl-gcc bazel run -c opt //ryu/benchmark:benchmark_fixed -- -samples=1000 -v -precision=$i -exp > musl-exp-$i.csv
#done

