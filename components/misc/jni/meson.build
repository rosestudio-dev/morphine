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

javahome_res = run_command(python, '-c', 'import os; print(os.environ["JAVA_HOME"], end="")').stdout()
jvm_cpu_family = target_machine.cpu()
jvm_cpu_family_alias = { 'x86_64' : 'amd64' }
if jvm_cpu_family in jvm_cpu_family_alias
    jvm_cpu_family = jvm_cpu_family_alias[jvm_cpu_family]
endif

cc = meson.get_compiler('c')
jni_dep = declare_dependency(
    include_directories : [
        javahome_res / 'include',
        javahome_res / 'include' / target_machine.system(),
    ],
    dependencies : cc.find_library(
        'jvm',
        dirs : [
            javahome_res / 'jre' / 'lib' / jvm_cpu_family / 'server',
            javahome_res / 'jre' / 'lib' / jvm_cpu_family
        ],
        required : true
    )
)

gradle = find_program('gradle', required : true)

custom_target(
    'morphinejni-jar',
    output : 'morphinejni.jar',
    console : true,
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
