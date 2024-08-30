# libs

subdir('math')
subdir('fs')

dependencies = [
    morphinelmath_dep,
    morphinelfs_dep
]

# dep

include = include_directories('include')

morphinel_dep = declare_dependency(
    include_directories : [include],
    dependencies : dependencies,
    version : meson.project_version()
)

install_subdir(
    'include',
    install_dir : 'include',
    strip_directory : true,
)
