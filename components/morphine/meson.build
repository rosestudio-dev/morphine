project(
    'morphine', 'c',
    default_options : ['c_std=c17']
)

installdir = get_option('installdir')
version = get_option('version')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

sources = run_command(
    'python',
    meson.source_root() / 'extra' / 'scripts' / 'source.py',
    check : true
).stdout().strip().split('\n')

private_include = include_directories('headers')
public_include = include_directories('include')

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
    '-DMORPHINE_VERSION="' + version + '"'
]

sanitize_flags = [
    '-fsanitize=address',
    '-fsanitize=undefined'
]

comiler_args = warning_flags + common_flags

if get_option('sanitize').enabled()
    comiler_args += sanitize_flags
endif

libmorphine = both_libraries(
    'libmorphine', sources,
    include_directories : [private_include, public_include],
    name_prefix : '',
    c_args : comiler_args,
    dependencies : m_dep,

    install : installdir != 'undefined',
    install_dir : installdir / 'libs'
)

libmorphine_dep = declare_dependency(
    include_directories : [public_include],
    link_with : libmorphine.get_static_lib(),
    version : version,
    dependencies : m_dep
)

if installdir != 'undefined'
    install_subdir(
        'include',
        follow_symlinks : false,
        install_dir : installdir / 'include',
        strip_directory : true,
    )
endif
