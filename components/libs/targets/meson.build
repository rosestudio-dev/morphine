libs = get_option('libs')
targets = []

foreach lib : libs
    subdir(lib)
    targets += {
        'name' : lib,
        'data' : get_variable('lib' + lib + '_data')
    }
endforeach
