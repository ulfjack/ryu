package(default_visibility=["//visibility:public"])

java_library(
  name = "analysis",
  srcs = glob(["*.java"]),
  deps = [
    "//src/main/java/info/adams/ryu",
    "//third_party/jaffer",
    "//third_party/mersenne_java",
  ],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "ComputeRequiredBitSizes",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "ExhaustiveFloatComparison",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "ExtensiveDoubleComparison",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "PrintFloatLookupTable",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "PrintDoubleLookupTable",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "ComputeTableSizes",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "FindDifferences",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "SlowConversion",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)

java_binary(
  name = "RandomizedDoubleComparison",
  runtime_deps = [":analysis"],
  # Travis CI: No Java support on Windows.
  tags = ["nowindows"],
)
