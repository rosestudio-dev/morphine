project(
    'app-morphine', 'c',
    default_options : ['c_std=c17']
)

installdir = get_option('installdir')

libmorphine = subproject(
    'morphine',
    default_options : ['installdir=' + installdir]
).get_variable('libmorphine_dep')

dlibcompiler_headers = subproject(
    'dlib-compiler',
    default_options : ['installdir=' + installdir]
).get_variable('dlibcompiler_headers_dep')

sources = run_command(
    'python',
    meson.source_root() / 'extra' / 'scripts' / 'source.py',
    'src',
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

comiler_args = warning_flags + common_flags

if get_option('sanitize').enabled()
    comiler_args += sanitize_flags
endif

executable(
    'morphine', sources,
    include_directories : include,
    dependencies : [libmorphine, dlibcompiler_headers],
    c_args : comiler_args,

    install : installdir != 'undefined',
    install_dir : installdir
)
