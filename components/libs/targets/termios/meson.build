include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphineltermios = library(
    'morphineltermios', sources,
    include_directories : [include],
    dependencies : [morphine_dep],
    c_args : c_args + ['-D_DEFAULT_SOURCE'],
    install : true,
)

morphineltermios_dep = declare_dependency(
    include_directories : [include],
    link_with : morphineltermios,
    version : meson.project_version()
)

libtermios_data = {
    'dep' : morphineltermios_dep,
    'include' : 'include'
}
