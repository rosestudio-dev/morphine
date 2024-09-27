
headers = include_directories('headers')
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

# configuration

subdir('include/morphine/config')

# dep

morphine = library(
    'morphine', sources, version_h_file, module_h_file,
    include_directories : [headers, include],
    c_args : c_args,
    link_args : link_args,
    c_shared_args : c_shared_flags,
    install : true,
)

morphine_dep = declare_dependency(
    include_directories : [include],
    link_with : morphine,
    version : meson.project_version()
)

install_subdir(
    'include',
    install_dir : 'include',
    exclude_files : [
        'morphine/config/meson.build',
        'morphine/config/version.h.in',
        'morphine/config/module.h.in'
    ],
    strip_directory : true,
)

install_data(
    version_h_file,
    install_dir : 'include/morphine/config'
)

install_data(
    module_h_file,
    install_dir : 'include/morphine/config'
)
