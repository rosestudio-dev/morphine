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
    '--no-daemon',
    '--project-dir',
    meson.current_source_dir(),
    '-DprojectVersion=' + version,
    '-DprojectVersionCode=' + versioncode,
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

nativeimage_options = [
    '--no-server',
    '--no-fallback',
]

# Targets

compiler_cli_jar = custom_target(
    'compiler-cli-jar',
    output : ['cli-' + version + '.jar'],
    console : true,
    command : [
        prog_gradle,
        gradle_options,
        ':cli:compiler:jar'
    ],
    depend_files : [sources_kts, sources_kt],

    install : false
)

compiler_clib_jar = custom_target(
    'compiler-clib-jar',
    output : ['clib-' + version + '.jar'],
    console : true,
    command : [
        prog_gradle,
        gradle_options,
        ':clibrary:jar'
    ],
    depend_files : [sources_kts, sources_kt],

    install : false
)

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
        prog_nativeimage,
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

# Dependencies

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

