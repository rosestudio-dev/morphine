project(
    'morphinec', 'c',
    default_options : ['c_std=c17']
)

installdir = get_option('installdir')
sanitize = get_option('sanitize')

prog_python = find_program('python', required : true)
extra_scripts = meson.source_root() / 'extra' / 'scripts'

morphine = subproject('morphine').get_variable('morphine_dep')

sources = run_command(
    prog_python,
    meson.source_root() / 'extra' / 'scripts' / 'walker.py',
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
    link_with : morphinec.get_static_lib()
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
            install_dir : installdir / 'cmake' / 'morphinec',
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
            install_dir : installdir / 'cmake' / 'morphinec'
        )
    endforeach

    extension = 'target_link_libraries(morphinec PRIVATE morphine)'

    custom_target(
        command : [
            prog_python,
            extra_scripts / 'cmake-gen.py',
            '-n',
            'morphinec',
            '-s',
            ':'.join(sources),
            '-i',
            'include',
            '-p',
            'headers',
            '-o',
            '@OUTPUT@',
            '-e',
            extension
        ],
        depend_files : sources + cmake_install_dirs + cmake_install_files,
        console : true,
        output : 'CMakeLists.txt',
        install : installdir != 'undefined',
        install_dir : installdir / 'cmake' / 'morphinec'
    )
endif

summary(
    {
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
