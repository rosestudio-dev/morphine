
# Defines

c_warning_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
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
    '-Wvla',
    '-Wmissing-prototypes',
    '-Wcast-qual',
    '-Woverflow',
]

c_common_flags = [
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
link_args = []

# Subdirs

buildapp = get_option('buildapp').enabled()
buildlibs = buildapp or get_option('buildlibs').enabled()
buildcompiler = buildapp or get_option('buildcompiler').enabled()

subdir('morphine')

if buildcompiler
    subdir('compiler')
endif

if buildlibs
    subdir('libs')
endif

if buildapp
    subdir('app')
endif
