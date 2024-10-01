# configuration

module_h_file = configure_file(
    input : 'module.h.in',
    output : 'module.h',
    configuration : {
        'MORPHINE_ENABLE_JUMPTABLE': get_option('morphine_module_jumptable').enabled(),
        'MORPHINE_ENABLE_SSO': get_option('morphine_module_sso').enabled(),
        'MORPHINE_ENABLE_DISTRIBUTED_GC': get_option('morphine_module_distributed_gc').enabled(),
        'MORPHINE_ENABLE_DEBUG': get_option('morphine_module_debug').enabled(),
    }
)

version_h_file = configure_file(
    input : 'version.h.in',
    output : 'version.h',
    configuration : {
        'MORPHINE_VERSION_NAME': '"' + meson.project_version() + '"',
        'MORPHINE_VERSION_CODE': version_code.to_string(),
        'MORPHINE_BYTECODE_VERSION': bytecode_version.to_string(),
    }
)
