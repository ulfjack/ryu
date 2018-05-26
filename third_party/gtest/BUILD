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
  linkopts = select({
    "@bazel_tools//src/conditions:windows": [],
    "//conditions:default": ["-lpthread"],
  }),
)
