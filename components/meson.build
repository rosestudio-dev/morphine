
# Defines

c_warning_flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wno-error=conversion',
    '-Wno-error=cast-align',
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

cpu = target_machine.cpu_family()
if cpu == 'wasm32' or cpu == 'wasm64'
    install_deps = false
else
    install_deps = true
endif

# Subdirs

subdir('morphine')
subdir('compiler')
subdir('libs')


if cpu == 'wasm32' or cpu == 'wasm64'
    subdir('app-wasm')
else
    subdir('app-cli')
endif
