include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelsubprocess = library(
    'morphinelsubprocess', sources,
    include_directories : [include],
    dependencies : [morphine_dep],
    c_args : c_args + ['-D_DEFAULT_SOURCE'],
    install : true,
)

morphinelsubprocess_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelsubprocess,
    version : meson.project_version()
)

libsubprocess_data = {
    'dep' : morphinelsubprocess_dep,
    'include' : 'include'
}
