load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ryu",
    srcs = [
        "common.h",
        "d2fixed.c",
        "d2fixed_full_table.h",
        "d2s.c",
        "d2s_full_table.h",
        "d2s_intrinsics.h",
        "d2s_small_table.h",
        "digit_table.h",
        "f2s.c",
        "f2s_full_table.h",
        "f2s_intrinsics.h",
    ],
    hdrs = ["ryu.h"],
)

cc_library(
    name = "ryu_parse",
    srcs = [
        "common.h",
        "d2s_full_table.h",
        "d2s_intrinsics.h",
        "d2s_small_table.h",
        "f2s_full_table.h",
        "f2s_intrinsics.h",
        "s2d.c",
        "s2f.c",
    ],
    hdrs = ["ryu_parse.h"],
)

cc_library(
    name = "generic_128",
    srcs = [
        "generic_128.c",
        "generic_128.h",
    ],
    hdrs = [
        "ryu_generic_128.h",
    ],
    # The code does not compile on Windows.
    tags = ["nowindows"],
)

# For testing only:

cc_library(
    name = "common",
    testonly = True,
    hdrs = [
        "common.h",
    ],
    visibility = ["//ryu/tests:__pkg__"],
)

cc_library(
    name = "d2s_intrinsics",
    testonly = True,
    hdrs = [
        "common.h",
        "d2s_intrinsics.h",
    ],
    visibility = ["//ryu/tests:__pkg__"],
)
