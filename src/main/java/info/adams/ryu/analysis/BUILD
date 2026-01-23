load("@rules_java//java:defs.bzl", "java_binary", "java_library")

package(default_visibility = ["//visibility:public"])

java_library(
    name = "analysis",
    srcs = glob(["*.java"]),
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    deps = [
        "//src/main/java/info/adams/ryu",
        "//third_party/jaffer",
        "//third_party/mersenne_java",
    ],
)

java_binary(
    name = "ComputeRequiredBitSizes",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "ExhaustiveFloatComparison",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "ExtensiveDoubleComparison",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "PrintFloatLookupTable",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "PrintDoubleLookupTable",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "ComputeTableSizes",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "FindDifferences",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "SlowConversion",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)

java_binary(
    name = "RandomizedDoubleComparison",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    runtime_deps = [":analysis"],
)
