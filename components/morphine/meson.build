project(
    'morphine', 'c',
    default_options : ['c_std=c17']
)

installdir = get_option('installdir')
version = get_option('version')
sanitize = get_option('sanitize')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

sources = run_command(
    'python',
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p',
    '**/*.c',
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

shared_flags = [
    '-DMORPHINE_LIBRARY'
]

compiler_args = warning_flags + common_flags

if sanitize.enabled()
    compiler_args += sanitize_flags
endif

morphine = both_libraries(
    'morphine', sources,
    include_directories : [private_include, public_include],
    c_args : compiler_args,
    dependencies : m_dep,
    c_shared_args : shared_flags,

    install : installdir != 'undefined',
    install_dir : installdir / 'libs'
)

morphine_dep = declare_dependency(
    include_directories : [public_include],
    link_with : morphine.get_static_lib(),
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

summary(
    {
        'version' : version,
        'install' : installdir != 'undefined',
        'install-dir' : installdir,
        'sanitize' : sanitize,
        'flags' : compiler_args,
        'sources' : sources
    },
    section : 'Configuration',
    bool_yn : true,
    list_sep : ', '
)
