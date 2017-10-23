import optparse
import sys
import os

from genacp import apply_template, parse_protocol

def parse_arguments():
    parser = optparse.OptionParser()
    parser.add_option("-t", "--type", dest='type',
                      help='type of generated file',
                      choices=('pxd', 'pyx'))
    parser.add_option("-o", "--output", dest='output',
                      help='write output to FILE', metavar='FILE')

    def vararg_callback(option, opt_str, value, parser):
        assert value is None
        value = []
        value.extend(parser.rargs)
        del parser.rargs[:len(value)]
        setattr(parser.values, option.dest, value)

    parser.add_option("-c",
                      "--cppflags", dest='cppflags',
                      help='additional CPP flags',
                      action="callback",
                      callback=vararg_callback
                     )

    options, input_files = parser.parse_args()
    options.input_files = input_files
    return options

script_dir = os.path.split(__file__)[0]
options = parse_arguments()

out = open(options.output, 'a')
try:
    if options.type == 'pyx':
        protocol = parse_protocol(
            options.input_files,
            options.cppflags
        )
        res = apply_template(
            os.path.join(script_dir, 'pyx.tmpl'),
            protocol
        )
    else:
        for input_file in options.input_files:
            protocol = parse_protocol(
                [input_file],
                options.cppflags
            )
            res = apply_template(
                os.path.join(script_dir, 'pxd.tmpl'),
                protocol
            )
            out.write(res)
    out.write(res)
finally:
    out.close()
