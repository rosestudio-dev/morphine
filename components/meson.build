
# Defines

if get_option('warning_as_error').enabled()
    warning_as_error_flag = [ '-Werror' ]
else
    warning_as_error_flag = [ ]
endif

c_warning_flags = warning_as_error_flag + [
    '-Wall',
    '-Wextra',
    '-Wno-error=conversion',
    '-Wno-error=cast-align',
    '-Wno-error=pointer-to-int-cast',
    '-Wdouble-promotion',
    '-Wshadow',
    '-Wformat',
    '-Wstrict-prototypes',
    '-Wunused-parameter',
    '-Wconversion',
    '-Wredundant-decls',
    '-Wmissing-declarations',
    '-Wmissing-field-initializers',
    '-Wvla',
    '-Wmissing-prototypes',
    '-Wcast-qual',
    '-Woverflow',
]

c_common_flags = [
    '-fvisibility=hidden',
    '-fno-common',
    '-fwrapv'
]

c_shared_flags = [
    '-DMORPHINE_LIBRARY'
]

script_path = meson.source_root() / 'extra' / 'scripts'
script_walker = script_path / 'walker.py'
script_version = script_path / 'version.py'
script_data = script_path / 'data.py'

# Complex

python = find_program('python', required : true)

c_args = c_warning_flags + c_common_flags

# Subdirs

build_misc = get_option('build_misc').enabled()
build_libs = get_option('build_libs').enabled()
build_compiler = get_option('build_compiler').enabled()

subdir('morphine')

if build_compiler
    subdir('compiler')
endif

if build_libs
    subdir('libs')
endif

if build_misc
    subdir('misc')
endif
