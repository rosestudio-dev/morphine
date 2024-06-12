project(
    'morphinec', 'c',
    default_options : ['c_std=c17']
)

version = get_option('version')
versioncode = get_option('versioncode')
installdir = get_option('installdir')
sanitize = get_option('sanitize')

morphine = subproject('morphine').get_variable('morphine_dep')

sources = run_command(
    'python',
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
    '-p',
    'src/**/*.c',
    check : true
).stdout().strip().split('\n')

public_include = include_directories('include')
private_include = include_directories('headers')

warning_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-error=conversion',
    '-Wno-error=strict-prototypes',
    '-Wdouble-promotion',
    '-Wshadow',
    '-Wformat',
    '-Wunused-parameter',
    '-Wconversion',
    '-Wredundant-decls',
    '-Wmissing-declarations',
    '-Wvla',
    '-Wmissing-prototypes',
    '-Wcast-qual',
    '-Wcast-align',
    '-Woverflow',
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

shared_flags = [
    '-DMORPHINE_LIBRARY'
]

compiler_args = warning_flags + common_flags

if sanitize.enabled()
    compiler_args += sanitize_flags
endif

morphinec = both_libraries(
    'morphinec', sources,
    include_directories : [private_include, public_include],
    c_args : compiler_args,
    dependencies : morphine,
    c_shared_args : shared_flags,

    install : installdir != 'undefined',
    install_dir : installdir / 'libs'
)

morphinec_dep = declare_dependency(
    include_directories : [public_include],
    link_with : morphinec.get_static_lib(),
    version : version
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
        'version_code' : versioncode,
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
