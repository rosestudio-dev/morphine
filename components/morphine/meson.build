project(
    'morphine', 'c',
    default_options : ['c_std=c17']
)

version_name = get_option('version-name')
version = get_option('version')
bytecode_version = get_option('bytecode-version')

installdir = get_option('installdir')
sanitize = get_option('sanitize')

prog_python = find_program('python', required : true)
extra_scripts = meson.source_root() / 'extra' / 'scripts'

sources = run_command(
    prog_python,
    extra_scripts / 'walker.py',
    '-p',
    'src/**/*.c',
    check : true
).stdout().strip().split('\n')

cmake_install_dirs = run_command(
    prog_python,
    extra_scripts / 'walker.py',
    '-d',
    'True',
    '-p',
    '*',
    check : true
).stdout().strip().split('\n')

cmake_install_files = run_command(
    prog_python,
    extra_scripts / 'walker.py',
    '-f',
    'True',
    '-p',
    '*',
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
    '-Woverflow',
]

common_flags = [
    '-fno-common',
    '-fwrapv'
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

mversions_target = custom_target(
    command : [
        prog_python,
        extra_scripts / 'version.py',
        '-n',
        version_name,
        '-v',
        version,
        '-b',
        bytecode_version,
        '-o',
        '@OUTPUT@'
    ],
    console : true,
    output : 'mversion.h',
    install : installdir != 'undefined',
    install_dir : installdir / 'cmake' / 'morphine' / 'headers'
)

mversions_dep = declare_dependency(
    sources : [mversions_target]
)

morphine = both_libraries(
    'morphine', sources,
    include_directories : [private_include, public_include],
    c_args : compiler_args,
    c_shared_args : shared_flags,
    dependencies : [mversions_dep],

    install : installdir != 'undefined',
    install_dir : installdir / 'libs'
)

morphine_dep = declare_dependency(
    include_directories : [public_include],
    link_with : morphine.get_static_lib(),
    version : version_name
)

if installdir != 'undefined'
    install_subdir(
        'include',
        follow_symlinks : false,
        install_dir : installdir / 'include',
        strip_directory : true,
    )

    foreach d : cmake_install_dirs
        if d.startswith('meson')
            continue
        endif

        install_subdir(
            d,
            follow_symlinks : false,
            install_dir : installdir / 'cmake' / 'morphine',
            strip_directory : false,
        )
    endforeach

    foreach f : cmake_install_files
        if f.startswith('meson')
            continue
        endif

        install_data(
            f,
            follow_symlinks : false,
            install_dir : installdir / 'cmake' / 'morphine'
        )
    endforeach

    custom_target(
        command : [
            prog_python,
            extra_scripts / 'cmake-gen.py',
            '-n',
            'morphine',
            '-s',
            ':'.join(sources),
            '-i',
            'include',
            '-p',
            'headers',
            '-o',
            '@OUTPUT@'
        ],
        depend_files : sources + cmake_install_dirs + cmake_install_files,
        console : true,
        output : 'CMakeLists.txt',
        install : installdir != 'undefined',
        install_dir : installdir / 'cmake' / 'morphine'
    )
endif

summary(
    {
        'version-name' : version_name,
        'version' : version,
        'bytecode-version' : bytecode_version,
        'install' : installdir != 'undefined',
        'install-dir' : installdir,
        'sanitize' : sanitize,
        'flags' : compiler_args,
        'sources' : sources,
        'cmake-dirs' : cmake_install_dirs,
        'cmake-files' : cmake_install_files,
    },
    section : 'Configuration',
    bool_yn : true,
    list_sep : ', '
)
