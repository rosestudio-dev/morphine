include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelsystem = library(
    'morphinelsystem', sources,
    include_directories : [include],
    dependencies : [morphine_dep],
    c_args : c_args,
    install : true,
)

morphinelsystem_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelsystem,
    version : meson.project_version()
)

libsystem_data = {
    'dep' : morphinelsystem_dep,
    'include' : 'include'
}
