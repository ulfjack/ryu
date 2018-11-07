package(default_visibility=["//visibility:public"])

# Build with --copt=-DMATCH_GRISU3_OUTPUT to match Grisu3 and check for
# differences.
cc_binary(
  name = "benchmark",
  srcs = ["benchmark.cc"],
  deps = [
    "//ryu",
    "//third_party/double-conversion",
  ],
)

cc_binary(
  name = "benchmark_fixed",
  srcs = ["benchmark_fixed.c"],
  deps = [
    "//ryu:ryu2",
    "//third_party/mersenne",
  ],
)
