bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1 > win-fixed-1.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=10 > win-fixed-10.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=100 > win-fixed-100.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1000 > win-fixed-1000.csv

bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1 > win-exp-1.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=10 > win-exp-10.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=100 > win-exp-100.csv
bazel run -c opt //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1000 > win-exp-1000.csv

/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1 > mingw-fixed-1.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=10 > mingw-fixed-10.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=100 > mingw-fixed-100.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -64 -samples=1000 -v -precision=1000 > mingw-fixed-1000.csv

/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1 > mingw-exp-1.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=10 > mingw-exp-10.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=100 > mingw-exp-100.csv
/c/Users/Ulf\ Adams/bin/bazel.exe --host_jvm_args="-Dbazel.windows_unix_root=C:/msys64/" run -c opt --compiler=msys-gcc //ryu/benchmark:benchmark_fixed_cc -- -exp -samples=1000 -v -precision=1000 > mingw-exp-1000.csv