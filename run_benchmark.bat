bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1 > win-fixed-1.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=10 > win-fixed-10.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=100 > win-fixed-100.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1000 > win-fixed-1000.csv

bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1 > win-exp-1.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=10 > win-exp-10.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=100 > win-exp-100.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1000 > win-exp-1000.csv