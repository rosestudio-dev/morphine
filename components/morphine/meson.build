
headers = include_directories('headers')
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

mversion_h = custom_target(
    command : [
        python, script_version,
        '-n', meson.project_version(),
        '-c', version_code.to_string(),
        '-b', bytecode_version.to_string(),
        '-o', '@OUTPUT@'
    ],
    console : true,
    output : 'mversion.h',
    install : install_deps,
    install_dir : 'include',
)

morphine = library(
    'morphine', sources, mversion_h,
    include_directories : [headers, include],
    c_args : c_args,
    c_shared_args : c_shared_flags,
    install : install_deps
)

morphine_dep = declare_dependency(
    include_directories : [include],
    link_with : morphine,
    version : meson.project_version()
)

if install_deps
    install_subdir(
        'include',
        install_dir : 'include',
        strip_directory : true,
    )
endif
