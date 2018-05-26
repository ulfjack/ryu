package(default_visibility=["//visibility:public"])

# Build with --copt=-DMATCH_GRISU3_OUTPUT to match Grisu3 and check for
# differences.
cc_binary(
  name = "benchmark",
  srcs = ["benchmark.cc"],
  deps = [
    "//ryu",
    "//third_party/mersenne",
    "//third_party/double-conversion",
  ],
)
