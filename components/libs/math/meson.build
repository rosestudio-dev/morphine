
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelmath = library(
    'morphinelmath', sources,
    include_directories : [include],
    dependencies : [morphine_dep, m_dep],
    c_args : c_args,
    c_shared_args : c_shared_flags,
    install : true,
)

morphinelmath_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelmath,
    version : meson.project_version()
)

install_subdir(
    'include',
    install_dir : 'include',
    strip_directory : true,
)
