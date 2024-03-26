project('dlib-compiler')

installdir = get_option('installdir')

compiler_clib_jar = subproject('compiler').get_variable('compiler_clib_jar')

nativeimage = find_program('native-image')
nativeimage_options = [
    '--no-server',
    '--no-fallback',
    '--parallelism=18',
]

dlibcompiler = custom_target(
    'dlib-compiler',
    input : [compiler_clib_jar],
    output : [
        'libcompiler.so',
        'libcompiler.h',
        'libcompiler_dynamic.h',
        'graal_isolate.h',
        'graal_isolate_dynamic.h'
    ],
    depends : [compiler_clib_jar],
    console : true,
    command : [
        nativeimage,
        nativeimage_options,
        '--shared',
        '-H:Name=libcompiler',
        '-jar',
        '@INPUT@',
        '-o',
        '@OUTDIR@' / 'libcompiler'
    ],

    install : installdir != 'undefined',
    install_dir : [
        installdir/'libs',
        installdir/'include'/'compiler',
        installdir/'include'/'compiler',
        installdir/'include'/'compiler',
        installdir/'include'/'compiler',
    ]
)

dlibcompiler_headers = []
dlibcompiler_shared = []

foreach s : dlibcompiler.to_list()
    if s.full_path().endswith('.h')
        dlibcompiler_headers += s
    else
        dlibcompiler_shared += s
    endif
endforeach

dlibcompiler_headers_dep = declare_dependency(
    sources : dlibcompiler_headers,
    version : get_option('version')
)

dlibcompiler_shared_dep = declare_dependency(
    sources : dlibcompiler_shared,
    version : get_option('version')
)
