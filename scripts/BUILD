CONVERSION_CMD_RYU="".join([
    "TMP_FILE=$$(mktemp /tmp/plot.XXXXXX)",
    " && cat $(location %s.template)",
    "  | sed \"s|INPUT_FILE|$<|g\"",
    "  | sed \"s|COMPARISON_NAME_1|%s|g\"",
    "  | sed \"s|COMPARISON_NAME_2|%s|g\"",
    "  > $${TMP_FILE}",
    " && gnuplot $${TMP_FILE} > $@",
    " && rm -f $${TMP_FILE}"])

[genrule(
  name = "shortest-" + t + "-" + f + "-csv",
  tools = [dep],
  outs = ["shortest-" + t + "-" + f + ".csv"],
  cmd = "$(location " + dep + ") " + o + " -classic -samples=1000 -v > $@",
) for (t,f,o,dep) in [
    ("c", "float", "-32", "//ryu/benchmark:ryu_benchmark"),
    ("c", "double", "-64", "//ryu/benchmark:ryu_benchmark"),
    ("java", "float", "-32", "//src/main/java/info/adams/ryu/benchmark"),
    ("java", "double", "-64", "//src/main/java/info/adams/ryu/benchmark"),
]]

[genrule(
  name = "shortest-" + t + "-" + f + "-pdf",
  srcs = ["shortest-" + t + "-" + f + ".csv"],
  tools = [f + ".template"],
  outs = ["shortest-" + t + "-" + f + ".pdf"],
  cmd = CONVERSION_CMD_RYU % (f, c, d),
) for (t,c,d) in [("c","Grisu3",""),("java","Jdk","Jaffer")] for f in ["float","double"]]



CONVERSION_CMD_RYU_PRINTF="".join([
    "TMP_FILE=$$(mktemp /tmp/plot.XXXXXX)",
    " && cat $(location %s.template)",
    "  | sed \"s|INPUT_FILE|$<|g\"",
    "  | sed \"s|COMPARISON_NAME_1|%s|g\"",
    "  > $${TMP_FILE}",
    " && gnuplot $${TMP_FILE} > $@",
    " && rm -f $${TMP_FILE}"])

[genrule(
  name = "f-" + t + "-" + f + "-" + p + "-csv",
  tools = [dep],
  outs = ["f-" + t + "-" + f + "-" + p + ".csv"],
  cmd = "$(location " + dep + ") " + o + " -classic -samples=2000 -precision=" + p + " -v > $@",
) for (t,f,o,dep) in [
    ("c", "double", "-f", "//ryu/benchmark:ryu_printf_benchmark"),
  ]
  for p in ["1", "10", "100", "1000"]
]

[genrule(
  name = "f-" + t + "-" + f + "-" + p + "-pdf",
  srcs = ["f-" + t + "-" + f + "-" + p + ".csv"],
  tools = ["printf.template"],
  outs = ["f-" + t + "-" + f + "-" + p + ".pdf"],
  cmd = CONVERSION_CMD_RYU_PRINTF % ("printf", c),
) for (t,c,d) in [("c","snprintf","")] for f in ["double"]
  for p in ["1", "10", "100", "1000"]
]

[genrule(
  name = "e-" + t + "-" + f + "-" + p + "-csv",
  tools = [dep],
  outs = ["e-" + t + "-" + f + "-" + p + ".csv"],
  cmd = "$(location " + dep + ") " + o + " -classic -samples=2000 -precision=" + p + " -v > $@",
) for (t,f,o,dep) in [
    ("c", "double", "-e", "//ryu/benchmark:ryu_printf_benchmark"),
  ]
  for p in ["1", "10", "100", "1000"]
]

[genrule(
  name = "e-" + t + "-" + f + "-" + p + "-pdf",
  srcs = ["e-" + t + "-" + f + "-" + p + ".csv"],
  tools = ["printf.template"],
  outs = ["e-" + t + "-" + f + "-" + p + ".pdf"],
  cmd = CONVERSION_CMD_RYU_PRINTF % ("printf", c),
) for (t,c,d) in [("c","snprintf","")] for f in ["double"]
  for p in ["1", "10", "100", "1000"]
]

