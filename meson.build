project(
    'morphine',
    default_options : [
        'version=0.0.1',
        'installdir=' + meson.source_root() / 'install',
        'morphine:sanitize=disabled',
        'app-morphine:sanitize=disabled',
        'compiler:localgradle=disabled',
    ],
    subproject_dir : 'components',
)

installdir = get_option('installdir')
message('Install dir:', installdir)

subproject('app-morphine', default_options : ['installdir=' + installdir])
subproject('app-compiler', default_options : ['installdir=' + installdir])

install_data(
    files('extra' / 'templates' / 'gitignore'),
    follow_symlinks : false,
    install_dir : installdir,
    rename : ['.gitignore'],
)
