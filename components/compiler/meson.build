project('compiler')

version = get_option('version')

gradle = find_program('gradle')
gradle_options = [
    '--no-daemon',
    '--project-dir',
    meson.current_source_dir(),
    '-DprojectVersion=' + version,
    '-DjarOutputDir=' + meson.current_build_dir()
]

if get_option('localgradle').enabled()
    gradle_options += [
        '-g',
        meson.build_root() / 'gradle' / '.gradle',
        '--project-cache-dir',
        meson.build_root() / 'gradle' / 'cache' / 'compiler',
        '-DmonoBuildDir=' + meson.build_root() / 'gradle' / 'build' / 'compiler',
    ]
endif

compiler_cli_jar = custom_target(
    'compiler-cli-jar',
    output : ['cli-' + version + '.jar'],
    console : true,
    command : [
        gradle,
        gradle_options,
        ':cli:compiler:jar'
    ],

    install : false
)

compiler_clib_jar = custom_target(
    'compiler-clib-jar',
    output : ['clib-' + version + '.jar'],
    console : true,
    command : [
        gradle,
        gradle_options,
        ':clibrary:jar'
    ],

    install : false
)