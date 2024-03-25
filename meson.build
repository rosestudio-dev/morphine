project(
    'morphine',
    default_options : [
        'morphine:sanitize=disabled',
        'morphine:debugger=disabled',
        'compiler:localgradle=disabled',
        'app-vm:sanitize=disabled',
        'version=0.0.1'
    ],
    subproject_dir : 'components',
)

components_version = meson.project_version()

subproject('app-compiler', default_options : ['version=' + components_version])
subproject('app-vm', default_options : ['version=' + components_version])
