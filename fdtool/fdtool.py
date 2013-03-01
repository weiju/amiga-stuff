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
import collections

# the format of a function definition
REGEX = re.compile('^([^()]+)\(([^()]*)\)\(([^()]*)\).*$')


class FDState:
    """represents the parser state"""
    def __init__(self, base, public, offset):
        self.base = base
        self.public = public
        self.offset = offset


def process_command(command, state):
    """process a FD command line"""
    if command.startswith('base'):
        state.base = command.split()[1]
    elif command.startswith('bias'):
        state.offset = int(command.split()[1])
    elif command.startswith('private'):
        state.public = False
    elif command.startswith('public'):
        state.public = True
    elif command.startswith('end'):
        pass
    else:
        print "unsupported command: ", command


def process_fundef(line, state):
    """process a FD function definition"""
    match = REGEX.match(line)
    if match:
        name, params, regs = match.group(1, 2, 3)
        if state.public:
            if len(params) > 0:
                print "-%d -> %s(%s), [%s]" % (state.offset,
                                               name, params, regs)
            else:
                print "-%d -> %s()" % (state.offset, name)
        state.offset += 6


def process(input_file):
    """process the specified input file"""
    state = FDState('', False, 0)
    with open(input_file) as infile:
        for line in infile:
            if line.startswith('*'):
                pass
            elif line.startswith('##'):
                process_command(line[2:], state)
            else:
                process_fundef(line.strip(), state)
    print "Done."

if __name__ == '__main__':
    description = """fdtool - stub generator for FD files (c) 2013"""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--infile', required=True, help="FD input file")
    args = parser.parse_args()
    process(args.infile)
