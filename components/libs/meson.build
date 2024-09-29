# configuration

subdir('targets')
subdir('include')
subdir('src')

# dep

dependencies = []

foreach target : targets
    dependencies += target['data']['dep']
endforeach

include = include_directories('include')

morphinel = library(
    'morphinel', import_c_file,
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

foreach target : targets
    install_subdir(
        'targets' / target['name'] / target['data']['include'],
        install_dir : 'include',
        strip_directory : true,
    )
endforeach
