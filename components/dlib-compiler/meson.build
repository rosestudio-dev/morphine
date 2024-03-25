project('dlib-compiler')

compiler_clib_jar = subproject('compiler').get_variable('compiler_clib_jar')

nativeimage = find_program('native-image')
nativeimage_options = [
    '--no-server',
    '--no-fallback',
]

dlib_compiler = custom_target(
    'dlib-compiler',
    input : [compiler_clib_jar],
    output : [
        'libmorphinecompiler.so',
        'libmorphinecompiler.h',
        'libmorphinecompiler_dynamic.h',
        'graal_isolate.h',
        'graal_isolate_dynamic.h'
    ],
    console : true,
    command : [
        nativeimage,
        nativeimage_options,
        '--shared',
        '-H:Name=libmorphinecompiler',
        '-jar',
        '@INPUT@',
        '-o',
        '@OUTDIR@/libmorphinecompiler'
    ]
)

dlib_compiler_dep = declare_dependency(
    sources : dlib_compiler,
    version: get_option('version')
)
