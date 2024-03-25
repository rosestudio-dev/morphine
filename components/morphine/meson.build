project(
    'morphine', 'c',
    default_options : ['c_std=c17']
)

version = get_option('version')

sources = run_command(
    'python',
    '../../source.py',
    check : true
).stdout().strip().split('\n')

private_include = include_directories('headers')
public_include = include_directories('include')

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

if get_option('debugger').enabled()
    comiler_args += ['-DMORPHINE_ENABLE_DEBUGGER']
endif

message('Compiler flags: ', comiler_args)
message('Sources: ', sources)

libmorphine = library(
    'libmorphine', sources,
    include_directories : [private_include, public_include],
    name_prefix : '',
    c_args : comiler_args,
    link_args : '-lm'
)

if get_option('debugger').enabled()
    libmorphine_dep = declare_dependency(
        include_directories : [private_include, public_include],
        link_with : libmorphine,
        link_args : '-lm',
        version : version
    )
else
    libmorphine_dep = declare_dependency(
        include_directories : [public_include],
        link_with : libmorphine,
        link_args : '-lm',
        version : version
    )
endif
