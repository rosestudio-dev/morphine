
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

headers = include_directories('headers')
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinel = library(
    'morphinel', sources,
    include_directories : [headers, include],
    dependencies : [morphine_dep, m_dep],
    c_args : c_args,
    c_shared_args : c_shared_flags,
    install : true
)

morphinel_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinel,
    version : meson.project_version()
)

install_subdir(
    'include',
    install_dir : 'include',
    strip_directory : true,
)
