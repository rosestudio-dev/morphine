
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

headers = include_directories('headers')
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelmath = library(
    'morphinelmath', sources,
    include_directories : [include, headers],
    dependencies : [morphine_dep, m_dep],
    c_args : c_args + [ '-D_DEFAULT_SOURCE'],
    link_args : link_args,
    c_shared_args : c_shared_flags,
    install : true,
)

morphinelmath_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelmath,
    version : meson.project_version()
)

libmath_data = {
    'dep' : morphinelmath_dep,
    'include' : 'include'
}

