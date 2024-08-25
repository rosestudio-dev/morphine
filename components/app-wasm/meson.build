
sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

executable(
    'morphine', sources,
    build_by_default : true,
    dependencies : [
        morphine_dep,
        morphinec_dep
    ],
    c_args : c_args + [
        '-Wno-error=missing-prototypes'
    ],
    link_args : [
        '-flto',
        '-sEXPORTED_FUNCTIONS=_morphine',
        '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap'
    ],
    install : true
)
