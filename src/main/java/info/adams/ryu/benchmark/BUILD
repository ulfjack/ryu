package(default_visibility=["//visibility:public"])

java_binary(
  name = "benchmark",
  main_class = "info.adams.ryu.benchmark.BenchmarkMain",
  srcs = glob(["*.java"]),
  deps = [
    "//src/main/java/info/adams/ryu",
    "//third_party/jaffer",
    "//third_party/mersenne_java",
  ],
)
