# Build gtest libs

gtest_srcs = [
    'gtest_main.cc',
    'gtest-all.cc',
]

libgtest = library('gtest', gtest_srcs,
    include_directories : base_include_dir,
    dependencies : dependency('threads'))

