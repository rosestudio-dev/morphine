
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinelbigint = library(
    'morphinelbigint', sources,
    include_directories : [include],
    dependencies : [morphine_dep],
    c_args : c_args,
    install : true,
)

morphinelbigint_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinelbigint,
    version : meson.project_version()
)

libbigint_data = {
    'dep' : morphinelbigint_dep,
    'include' : 'include'
}
