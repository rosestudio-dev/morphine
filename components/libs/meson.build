# options

uselib_math = get_option('uselib_math').enabled()
uselib_fs = get_option('uselib_fs').enabled()
uselib_system = get_option('uselib_system').enabled()
uselib_bigint = get_option('uselib_bigint').enabled()

# libs

dependencies = []

if uselib_math
    subdir('math')
    dependencies += morphinelmath_dep
endif

if uselib_fs
    subdir('fs')
    dependencies += morphinelfs_dep
endif

if uselib_system
    subdir('system')
    dependencies += morphinelsystem_dep
endif

if uselib_bigint
    subdir('bigint')
    dependencies += morphinelbigint_dep
endif

# configuration

subdir('include')

# dep

include = include_directories('include')

sources = run_command(
    python, script_walker, '-p', 'src/**/*.c',
    check : true
).stdout().strip().split('\n')

morphinel = library(
    'morphinel', sources,
    include_directories : [include],
    dependencies : [
        dependencies,
        morphine_dep
    ],
    c_args : c_args,
    link_args : link_args,
    c_shared_args : c_shared_flags,
    install : true,
)

morphinel_dep = declare_dependency(
    include_directories : [include],
    link_with : morphinel,
    dependencies : dependencies,
    version : meson.project_version()
)

install_subdir(
    'include',
    install_dir : 'include',
    exclude_files : [
        'meson.build',
        'morphinel.h.in'
    ],
    strip_directory : true,
)

install_data(
    morphinel_h_file,
    install_dir : 'include'
)
