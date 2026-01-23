load("@rules_java//java:defs.bzl", "java_binary")

package(default_visibility = ["//visibility:public"])

java_binary(
    name = "benchmark",
    srcs = glob(["*.java"]),
    main_class = "info.adams.ryu.benchmark.BenchmarkMain",
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    deps = [
        "//src/main/java/info/adams/ryu",
        "//third_party/jaffer",
        "//third_party/mersenne_java",
    ],
)
