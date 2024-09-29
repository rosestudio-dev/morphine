imports_source = ''

foreach target : targets
    imports_source += '\n    mapi_library_load(I, mllib_' + target['name'] + '());'
endforeach

import_c_file = configure_file(
    input : 'import.c.in',
    output : 'import.c',
    configuration : { 'imports': imports_source }
)
