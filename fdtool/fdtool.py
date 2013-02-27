#!/usr/bin/python
"""
fdtool.py - simple FD file processing tool.

This is an alternative to fd2pragma, written in Python.
It was written to understand the FD format and its relationship
with programming languages and to take advantage of the strengths
of a scripting language.
"""
import argparse
import re

# the format of a function definition
REGEX = re.compile('^([^()]+)\(([^()]*)\)\(([^()]*)\).*$')

fd_settings = {}
offset = 0
is_public = False


def process_command(command):
    """process a FD command line"""
    global fd_settings, is_public, offset
    if command.startswith('base'):
        comps = command.split()
        fd_settings[comps[0]] = comps[1]
    elif command.startswith('bias'):
        if 'bias' in fd_settings:
            raise Exception('multiple bias entries not yet supported')
        else:
            comps = command.split()
            offset = int(comps[1])
            fd_settings[comps[0]] = offset
    elif command.startswith('private'):
        is_public = False
    elif command.startswith('public'):
        is_public = True
    elif command.startswith('end'):
        pass
    else:
        print "unsupported command: ", command


def process_fundef(line):
    """process a FD function definition"""
    global offset
    match = REGEX.match(line)
    if match:
        name, params, regs = match.group(1, 2, 3)
        if is_public:
            if len(params) > 0:
                print "-%d -> %s(%s), [%s]" % (offset, name, params, regs)
            else:
                print "-%d -> %s()" % (offset, name)
        offset += 6


def process(input_file, output_dir):
    """process the specified input file"""
    with open(input_file) as infile:
        for line in infile:
            if line.startswith('*'):
                pass
            elif line.startswith('##'):
                process_command(line[2:])
            else:
                process_fundef(line.strip())
    print "Done."

if __name__ == '__main__':
    description = """fdtool - stub generator for FD files (c) 2013"""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--infile', required=True, help="FD input file")
    parser.add_argument('--outdir', required=True, help="output directory")
    args = parser.parse_args()
    process(args.infile, args.outdir)
