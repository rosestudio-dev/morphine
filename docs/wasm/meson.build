project(
    'morphine-wasm', 'c',
    default_options : {
        'c_std' : 'c17',
        'prefix' : meson.source_root() / 'install',
    }
)

cc = meson.get_compiler('c')

executable(
    'morphine', 'src/main.c',
    build_by_default : true,
    dependencies : [
        cc.find_library(
            'libmorphine',
            dirs : [meson.project_source_root() / 'deps' / 'lib'],
            required : true,
        ),
        cc.find_library(
            'libmorphinec',
            dirs : [meson.project_source_root() / 'deps' / 'lib'],
            required : true,
        ),
        cc.find_library(
            'libmorphinel',
            dirs : [meson.project_source_root() / 'deps' / 'lib'],
            required : true,
        ),
        cc.find_library(
            'libmorphinelbigint',
            dirs : [meson.project_source_root() / 'deps' / 'lib'],
            required : true,
        ),
    ],
    include_directories : ['deps' / 'include'],
    link_args : [
        '-flto',
        '-sEXPORTED_FUNCTIONS=_morphine',
        '-sEXPORTED_RUNTIME_METHODS=ccall,cwrap',
        '-sMODULARIZE=1',
        '-sEXPORT_ES6=1',
        '-sALLOW_MEMORY_GROWTH=1'
    ],
    install : true
)
