includes_source = ''

foreach target : targets
    includes_source += '#include <morphinel/' + target['name'] + '.h>\n'
endforeach

morphinel_h_file = configure_file(
    input : 'morphinel.h.in',
    output : 'morphinel.h',
    configuration : { 'includes': includes_source }
)
