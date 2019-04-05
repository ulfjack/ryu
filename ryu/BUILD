package(default_visibility=["//visibility:public"])

cc_library(
  name = "ryu",
  srcs = [
    "f2s.c",
    "d2s.c",
    "d2s.h",
    "d2s_full_table.h",
    "d2s_intrinsics.h",
    "digit_table.h",
    "common.h",
    "config.h",
  ],
  hdrs = ["ryu.h", "ryu.hpp"],
)

cc_library(
  name = "ryu_header_only",
  srcs = [],
  hdrs = [
    "d2s.h",
    "d2s.c",
    "f2s.c",
    "d2s_full_table.h",
    "d2s_intrinsics.h",
    "digit_table.h",
    "common.h",
    "config.h",
    "ryu_header_only.hpp",
    "ryu.h",
  ],
)

cc_library(
  name = "ryu2",
  srcs = [
    "d2fixed.c",
#    "d2fixed.h",
    "d2s_intrinsics.h",
    "d2fixed_full_table.h",
    "digit_table.h",
    "common.h",
    "config.h",
  ],
  hdrs = ["ryu2.h", "ryu2.hpp"],
)
cc_library(
  name = "ryu2_header_only",
  srcs = [],
  hdrs = [
    "d2s_intrinsics.h",
    "d2fixed_full_table.h",
    "digit_table.h",
    "common.h",
    "config.h",
    "ryu2.h",
    "ryu2_header_only.hpp",
    "d2fixed.c",
  ],
)


cc_library(
  name = "generic_128",
  srcs = [
    "generic_128.c",
    "generic_128.h",
    "config.h",
  ],
  hdrs = [
    "ryu_generic_128.h",
    "ryu_generic_128.hpp",
  ],
)

cc_library(
  name = "generic_128_header_only",
  srcs = [],
  hdrs = [
    "generic_128.c",
    "generic_128.h",
    "config.h",
    "ryu_generic_128.h",
    "ryu_generic_128_header_only.hpp",
  ],
)

cc_library(
  name = "d2s_intrinsics",
  hdrs = [
    "common.h",
    "d2s_intrinsics.h",
    "config.h",
  ],
)
