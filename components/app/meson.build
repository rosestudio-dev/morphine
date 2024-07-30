
include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

executable(
    'morphine', sources,
    include_directories : include,
    dependencies : [morphine_dep, morphinec_dep, morphinel_dep],
    c_args : c_args,
    link_args : '-flto',
    install : true
)
