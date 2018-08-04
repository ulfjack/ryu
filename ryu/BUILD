package(default_visibility=["//visibility:public"])

cc_library(
  name = "ryu",
  srcs = [
    "f2s.c",
    "d2s.c",
    "d2s.h",
    "d2s_full_table.h",
    "digit_table.h",
    "mulshift128.h",
    "common.h",
  ],
  hdrs = [
    "ryu.h",
    # Experimental low-level API; may change at any time.
    "ryu_low_level.h",
  ],
)
