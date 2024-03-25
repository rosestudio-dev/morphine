project(
    'app-vm', 'c',
    default_options : ['c_std=c17']
)

libmorphine = subproject(
    'morphine',
    default_options : [
        'default_library=static'
    ]
).get_variable('libmorphine_dep')

dlib_compiler_headers = subproject('dlib-compiler').get_variable('dlib_compiler_headers_dep')

sources = run_command(
    'python',
    '..'/'..'/'source.py',
    'src',
    check : true
).stdout().strip().split('\n')

include = include_directories('include')

warning_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-error=conversion',
    '-Wdouble-promotion',
    '-Wshadow',
    '-Wformat',
    '-Wstrict-prototypes',
    '-Wunused-parameter',
    '-Wconversion',
    '-Wredundant-decls',
    '-Wmissing-declarations',
    '-Wvla',
    '-Wmissing-prototypes',
    '-Wcast-qual',
    '-Wcast-align',
]

common_flags = [
    '-fno-common',
    '-fwrapv',
    '-flto'
]

sanitize_flags = [
    '-fsanitize=address',
    '-fsanitize=undefined'
]

comiler_args = warning_flags + common_flags

if get_option('sanitize').enabled()
    comiler_args += sanitize_flags
endif

message('Compiler flags: ', comiler_args)
message('Sources: ', sources)

executable(
    'morphine', sources,
    include_directories : include,
    dependencies : [libmorphine, dlib_compiler_headers],
    c_args : comiler_args
)
