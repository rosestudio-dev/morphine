project('compiler')

version = get_option('version')
versioncode = get_option('versioncode')
installdir = get_option('installdir')

prog_nativeimage = find_program('native-image')
prog_gradle = find_program('gradle')
prog_python = find_program('python', required : true)

# Sources

sources_kts = run_command(
    prog_python,
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p',
    '**/*.kt',
    check : true
).stdout().strip().split('\n')

sources_kt = run_command(
    prog_python,
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p',
    '**/*.kts',
    check : true
).stdout().strip().split('\n')

# Options

gradle_options = [
    '--project-dir',
    meson.current_source_dir(),
    '-DprojectVersion=' + version,
    '-DprojectVersionCode=' + versioncode,
    '-DmonoBuildDir=' + meson.global_build_root() / 'gradleBuild',
    '-DoutputDir='+meson.current_build_dir(),
]

if get_option('localgradle').enabled()
    gradle_options += [
        '-g',
        meson.global_build_root() / 'gradle' / '.gradle',
        '--project-cache-dir',
        meson.global_build_root() / 'gradle' / 'cache' / 'compiler',
    ]
endif

if get_option('gradledaemon').disabled()
    gradle_options += ['--no-daemon']
endif

nativeimage_options = [
    '--no-server',
    '--no-fallback',
]

# Targets

compiler_output = [
    'cli.kexe',
    'libsharedcompiler.so',
    'libsharedcompiler_api.h',
    'libstaticcompiler.a',
    'libstaticcompiler_api.h',
    'library-jvm-' + version + '.jar'
]

compiler_shadow_output = [
    'compiler',
    'libcompiler.so',
    'libcompiler_shared_api.h',
    'libcompiler.a',
    'libcompiler_static_api.h',
    'compiler.jar'
]

compiler_output_parsed = []
compiler_shadow_output_parsed = []

foreach s : compiler_output
    compiler_output_parsed += ['@OUTDIR@' / s]
endforeach

foreach s : compiler_shadow_output
    compiler_shadow_output_parsed += ['@OUTDIR@' / s]
endforeach

compiler_raw = custom_target(
    'compiler-raw',
    output : compiler_output,
    console : true,
    command : [
        prog_gradle,
        gradle_options,
        'production'
    ],
    depend_files : [sources_kts, sources_kt],

    install : false
)

compiler = custom_target(
    'compiler',
    output : compiler_shadow_output,
    console : true,
    command : [
        prog_python,
        meson.source_root() / 'extra' / 'scripts' / 'rename.py',
        '-i',
        ':'.join(compiler_output_parsed),
        '-o',
        ':'.join(compiler_shadow_output_parsed),
    ],
    depends : [compiler_raw],

    install : installdir != 'undefined',
    install_dir : [
        installdir,
        installdir / 'libs',
        installdir / 'include' / 'compiler',
        installdir / 'libs',
        installdir / 'include' / 'compiler',
        installdir / 'libs',
    ]
)

# Dependencies

compiler_lib_static_dep = declare_dependency(
    sources : [compiler[3], compiler[4]],
    version : get_option('version'),
    link_with : [compiler[3]]
)

compiler_lib_shared_dep = declare_dependency(
    sources : [compiler[2]],
    version : get_option('version'),
)

# Misc

summary(
    {
        'version' : version,
        'version_code' : versioncode,
        'sources' : [sources_kts, sources_kt],
    },
    section : 'Configuration',
    bool_yn : true,
    list_sep : ', '
)

