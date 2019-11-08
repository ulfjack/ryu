package(default_visibility=["//visibility:public"])

# Build with --copt=-DMATCH_GRISU3_OUTPUT to match Grisu3 and check for
# differences.
cc_binary(
  name = "ryu_benchmark",
  srcs = ["benchmark.cc"],
  deps = [
    "//ryu",
    "//third_party/double-conversion",
  ],
)

cc_binary(
  name = "ryu_printf_benchmark_c",
  srcs = ["benchmark_fixed.c"],
  deps = [
    "//ryu",
    "//third_party/mersenne",
  ],
)

cc_binary(
  name = "ryu_printf_benchmark",
  srcs = ["benchmark_fixed.cc"],
  deps = [
    "//ryu",
    "//third_party/mersenne",
  ],
)
