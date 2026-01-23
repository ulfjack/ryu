load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "gtest",
    srcs = ["gtest_main.cc"],
    deps = [":gtest-lib"],
)

cc_library(
    name = "gtest-lib",
    srcs = ["gtest-all.cc"],
    hdrs = ["gtest.h"],
    defines = ["GTEST_LANG_CXX11=1"],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [],
        "//conditions:default": ["-lpthread"],
    }),
)
