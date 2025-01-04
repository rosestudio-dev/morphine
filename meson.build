project(
    'morphine', 'c',
    version : '0.1-1dev',
    license : 'MIT',
    license_files : ['LICENSE'],
    default_options : {
        'c_std' : 'c17',
        'cpp_std' : 'c++17',
        'default_library' : 'static',
        'prefix' : meson.project_build_root() / 'output',
    }
)

version_code = 1
bytecode_version = 1

subdir('components')
