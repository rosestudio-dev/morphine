sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

kt_sources = run_command(
    python, script_walker, '-p', 'javalib/**/*.kt',
    check : true
).stdout().strip().split('\n')

kts_sources = run_command(
    python, script_walker, '-p', 'javalib/**/*.kts',
    check : true
).stdout().strip().split('\n')

jni_dep = dependency('jni')

gradle = find_program('gradle', required : true)

custom_target(
    'morphinejni-jar',
    output : 'morphinejni.jar',
    console: true,
    command : [
        gradle,
        '--no-daemon',
        '--project-cache-dir', meson.current_build_dir() / 'gradle-project',
        '--gradle-user-home', meson.current_build_dir() / 'gradle-home',
        '-PjarOutputPath=' + meson.current_build_dir(),
        '-p', meson.current_source_dir() / 'javalib',
        'jar'
    ],
    depend_files : kt_sources + kts_sources,
    install : true,
    install_dir : 'lib'
)

morphinejni = shared_library(
    'morphinejni', sources,
    include_directories : ['headers'],
    dependencies : [
        morphine_dep,
        morphinec_dep,
        morphinel_dep,
        jni_dep,
    ],
    c_args : c_args,
    link_args : link_args,
    install : true
)
