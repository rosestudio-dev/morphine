# configuration

morphinel_h_file = configure_file(
    input : 'morphinel.h.in',
    output : 'morphinel.h',
    configuration : {
        'MORPHINEL_USE_MATH': uselib_math,
        'MORPHINEL_USE_FS': uselib_fs,
        'MORPHINEL_USE_SYSTEM': uselib_system,
        'MORPHINEL_USE_BIGINT': uselib_bigint,
    }
)
