# The Ryu library

srcs = [
    'f2s.c',
    'd2s.c',
]

install_headers(files('ryu.h'))

# Plain Ryu (f2s, d2s)
libryu = library('ryu', srcs,
    include_directories : base_include_dir,
    install : true)

# Ryu printf (d2fixed)
libryu_printf = library('ryu_printf', 'd2fixed.c',
    include_directories : base_include_dir,
    install : true)

# 128bit helpers
libgen128 = library('generic_128', 'generic_128.c',
    include_directories : base_include_dir,
    install : false)

subdir('tests')

# Some simple example snippet
example = executable('example', 'example.c',
    link_with : [ libryu, libryu_printf ] )

