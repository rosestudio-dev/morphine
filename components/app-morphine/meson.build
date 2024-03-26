project(
    'app-morphine', 'c',
    default_options : ['c_std=c17']
)

version = get_option('version')
installdir = get_option('installdir')

morphine = subproject('morphine').get_variable('morphine_dep')
dlibcompiler_headers = subproject('dlib-compiler').get_variable('dlibcompiler_headers_dep')

sources = run_command(
    'python',
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p',
    'src/**/*.c',
    check : true
).stdout().strip().split('\n')

include = include_directories('include')

warning_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-error=conversion',
    '-Wdouble-promotion',
    '-Wshadow',
    '-Wformat',
    '-Wstrict-prototypes',
    '-Wunused-parameter',
    '-Wconversion',
    '-Wredundant-decls',
    '-Wmissing-declarations',
    '-Wvla',
    '-Wmissing-prototypes',
    '-Wcast-qual',
    '-Wcast-align',
]

common_flags = [
    '-fno-common',
    '-fwrapv',
    '-flto'
]

sanitize_flags = [
    '-fsanitize=address',
    '-fsanitize=undefined'
]

compiler_args = warning_flags + common_flags

if get_option('sanitize').enabled()
    compiler_args += sanitize_flags
endif

executable(
    'morphine', sources,
    include_directories : include,
    dependencies : [morphine, dlibcompiler_headers],
    c_args : compiler_args,

    install : installdir != 'undefined',
    install_dir : installdir
)

summary(
    {
        'version' : version,
        'install' : installdir != 'undefined',
        'install-dir' : installdir,
        'flags' : compiler_args,
        'sources' : sources
    },
    section : 'Configuration',
    bool_yn : true
)
