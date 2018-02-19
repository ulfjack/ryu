package(default_visibility=["//visibility:public"])

java_library(
  name = "analysis",
  srcs = glob(["*.java"]),
  deps = [
    "//src/main/java/info/adams/ryu",
  ],
)

java_binary(
  name = "ComputeRequiredBitSizes",
  runtime_deps = [":analysis"],
)

java_binary(
  name = "ExhaustiveFloatComparison",
  runtime_deps = [":analysis"],
)

java_binary(
  name = "PrintFloatLookupTable",
  runtime_deps = [":analysis"],
)

java_binary(
  name = "PrintDoubleLookupTable",
  runtime_deps = [":analysis"],
)

java_binary(
  name = "ComputeTableSizes",
  runtime_deps = [":analysis"],
)