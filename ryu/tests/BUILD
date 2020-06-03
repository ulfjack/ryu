
cc_test(
  name = "common_test",
  srcs = ["common_test.cc"],
  deps = [
    "//ryu:common",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "d2s_intrinsics_test",
  srcs = ["d2s_intrinsics_test.cc"],
  deps = [
    "//ryu:d2s_intrinsics",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "d2s_test",
  srcs = ["d2s_test.cc"],
  deps = [
    "//ryu",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "s2d_test",
  srcs = ["s2d_test.cc"],
  deps = [
    "//ryu:ryu_parse",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "d2s_table_test",
  srcs = ["d2s_table_test.cc"],
  deps = [
    "//ryu",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "f2s_test",
  srcs = ["f2s_test.cc"],
  deps = [
    "//ryu",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "s2f_test",
  srcs = ["s2f_test.cc"],
  deps = [
    "//ryu:ryu_parse",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "ryu_printf_test",
  srcs = ["d2fixed_test.cc"],
  deps = [
    "//ryu",
    "//third_party/gtest",
  ],
)

cc_test(
  name = "generic_128_test",
  srcs = ["generic_128_test.cc"],
  # The code does not run on Windows yet.
  tags = ["nowindows"],
  deps = [
    "//ryu:generic_128",
    "//third_party/gtest",
  ],
)

