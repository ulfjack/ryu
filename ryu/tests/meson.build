# Build Ryu tests

test_src = [
    'd2s_intrinsics_test.cc',
    'd2s_table_test.cc',
    'd2s_test.cc',
    'f2s_test.cc',
    'd2fixed_test.cc',
    'generic_128_test.cc',
]

testryu = executable('testryu', test_src,
    include_directories : base_include_dir,
    link_with : [ libryu, libryu_printf, libgen128, libgtest ] )

test('all', testryu)
