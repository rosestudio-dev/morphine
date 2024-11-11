
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelfs = library(
    'morphinelfs', sources,
    include_directories : [include],
    dependencies : [morphine_dep],
    c_args : c_args + [ '-D_XOPEN_SOURCE=700'],
    link_args : link_args,
    c_shared_args : c_shared_flags,
    install : true,
)

morphinelfs_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelfs,
    version : meson.project_version()
)

libfs_data = {
    'dep' : morphinelfs_dep,
    'include' : 'include'
}
