project('app-compiler')

installdir = get_option('installdir')

compiler_cli_jar = subproject('compiler').get_variable('compiler_cli_jar')

nativeimage = find_program('native-image')
nativeimage_options = [
    '--no-server',
    '--no-fallback',
    '--parallelism=18',
]

custom_target(
    'compiler',
    input : [compiler_cli_jar],
    output : ['compiler'],
    console : true,
    command : [
        nativeimage,
        nativeimage_options,
        '-jar',
        '@INPUT@',
        '-o',
        '@OUTPUT@'
    ],

    install : installdir != 'undefined',
    install_dir : installdir
)
