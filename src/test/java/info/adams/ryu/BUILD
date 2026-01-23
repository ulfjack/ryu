load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//visibility:public"])

java_test(
    name = "ryu",
    srcs = glob(["*.java"]),
    # Travis CI: No Java support on Windows.
    tags = ["nowindows"],
    test_class = "info.adams.ryu.AllTests",
    deps = [
        "//src/main/java/info/adams/ryu",
        "//src/main/java/info/adams/ryu/analysis",
        "//third_party/jaffer",
        "//third_party/junit",
    ],
)
