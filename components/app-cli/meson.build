
sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphine_scripts_paths = run_command(
    python, script_walker, '-p', 'scripts/**/*.mos',
    check : true
).stdout().strip().split('\n')

morphine_scripts = custom_target(
    command : [
        python, script_data,
        '-s', meson.current_source_dir(),
        '-f', ':'.join(morphine_scripts_paths),
        '-p', 'scripts/',
        '-o', '@OUTPUT@'
    ],
    input : morphine_scripts_paths,
    output : 'scripts.h'
)

executable(
    'morphine', sources, morphine_scripts,
    build_by_default : true,
    dependencies : [
        morphine_dep,
        morphinec_dep,
        morphinel_dep
    ],
    c_args : c_args,
    link_args : '-flto',
    install : true
)
