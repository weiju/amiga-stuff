#!/usr/bin/env python3
"""A tool like dalf.rexx that can be used to inspect the structure of
an Amiga Hunk file.
"""
import os
import argparse
import struct

from disassemble import disassemble

HUNK_BLOCK_UNIT         = b'\x00\x00\x03\xe7'
HUNK_BLOCK_NAME         = b'\x00\x00\x03\xe8'
HUNK_BLOCK_CODE         = b'\x00\x00\x03\xe9'
HUNK_BLOCK_DATA         = b'\x00\x00\x03\xea'
HUNK_BLOCK_BSS          = b'\x00\x00\x03\xeb'
HUNK_BLOCK_RELOC32      = b'\x00\x00\x03\xec'
HUNK_BLOCK_RELOC16      = b'\x00\x00\x03\xed'
HUNK_BLOCK_RELOC8       = b'\x00\x00\x03\xee'
HUNK_BLOCK_EXT          = b'\x00\x00\x03\xef'
HUNK_BLOCK_SYMBOL       = b'\x00\x00\x03\xf0'
HUNK_BLOCK_DEBUG        = b'\x00\x00\x03\xf1'
HUNK_BLOCK_END          = b'\x00\x00\x03\xf2'
HUNK_BLOCK_HEADER       = b'\x00\x00\x03\xf3'
HUNK_BLOCK_OVERLAY      = b'\x00\x00\x03\xf5'
HUNK_BLOCK_BREAK        = b'\x00\x00\x03\xf6'
HUNK_BLOCK_DRELOC32     = b'\x00\x00\x03\xf7'
HUNK_BLOCK_DRELOC16     = b'\x00\x00\x03\xf8'
HUNK_BLOCK_DRELOC8      = b'\x00\x00\x03\xf9'
HUNK_BLOCK_RELOC32SHORT = b'\x00\x00\x03\xfc'

# new library file
HUNK_BLOCK_LIB          = b'\x00\x00\x03\xfb'
HUNK_BLOCK_INDEX        = b'\x00\x00\x03\xfc'


def read_int32(infile):
    return struct.unpack('>i', infile.read(4))[0]


def read_uint32(infile):
    return struct.unpack('>I', infile.read(4))[0]

def read_int16(infile):
    return struct.unpack('>h', infile.read(2))[0]


def read_uint16(infile):
    return struct.unpack('>H', infile.read(2))[0]

def read_string_list(infile):
    result = []
    strlen = read_int32(infile)
    while strlen > 0:
        result.append(str(infile.read(strlen)))
        strlen = read_int32(infile)

    return []


def read_string(infile):
    result = None
    strlen = read_uint32(infile) * 4
    if strlen == 0:
        return None
    result = str(infile.read(strlen))
    idx = result.find("\0")
    return result[:idx]


def read_symbols(infile):
    result = []
    symbol = read_string(infile)
    while symbol is not None:
        offset = read_int32(infile)  # actually uint32
        result.append({"symbol": symbol, "offset": offset})
        symbol = read_string(infile)
    return result


def read_raw_bytes(infile):
    num_bytes = read_int32(infile) * 4
    data = infile.read(num_bytes)
    if len(data) < num_bytes:
        print("WARNING: tried to read %d bytes, but only %d bytes could be read" % (num_bytes,
                                                                                    bytes_read))
    return data


def read_reloc32map(infile):
    num_offsets = read_int32(infile)
    result = {}
    while num_offsets > 0:
        hunknum = read_int32(infile)
        hunk_offsets = [read_int32(infile) for i in range(num_offsets)]
        result[hunknum] = hunk_offsets
        num_offsets = read_int32(infile)
    return result


def read_reloc16map(infile):
    num_offsets = read_int16(infile)
    total_offsets = num_offsets

    result = {}
    while num_offsets > 0:
        hunknum = read_int16(infile)
        hunk_offsets = [read_int16(infile) for i in range(num_offsets)]
        result[hunknum] = hunk_offsets
        num_offsets = read_int16(infile)
        total_offsets += num_offsets

    if (total_offsets % 2) == 0:
        read_int16(infile)

    return result


def read_block(infile, is_loadfile):
    id = infile.read(4)
    if len(id) == 0:  # end of input
        return None

    if id == HUNK_BLOCK_CODE:
        return ('CODE', read_raw_bytes(infile))
    elif id == HUNK_BLOCK_RELOC32:
        return ('RELOC32', read_reloc32map(infile))
    elif id == HUNK_BLOCK_END:
        return ('END', None)
    elif id == HUNK_BLOCK_DRELOC32:
        if is_loadfile:
            return ('RELOC32SHORT', read_reloc16map(infile))
        else:
            return ('RELOC32', read_reloc32map(infile))
    elif id == HUNK_BLOCK_DATA:
        return ('DATA', read_raw_bytes(infile))
    elif id == HUNK_BLOCK_BSS:
        return ('BSS', read_int32(infile) * 4)
    elif id == HUNK_BLOCK_NAME:
        return ('NAME', str(infile.read(read_int32(infile) * 4)))
    elif id == HUNK_BLOCK_SYMBOL:
        return ('SYMBOL', read_symbols(infile))
    else:
        raise Exception("unsupported id: %s" % id)

    return None


def read_blocks(infile, is_loadfile):
    result = []
    block = read_block(infile, is_loadfile)
    while block is not None:
        result.append(block)
        block = read_block(infile, is_loadfile)
    return result


def group_blocks(blocks):
    """group the blocks into relocation groups"""
    groups = []
    for block in blocks:
        if block[0] in {'NAME', 'UNIT'}:
            continue
        elif block[0] in {'BSS', 'CODE', 'DATA'}:
            current_group = []

        if block[0] == 'END':
            groups.append(current_group)
            current_group = []
        else:
            current_group.append(block)
    return groups


def print_data(data, items_per_line=16):
    """Do a hex dump on the specified data"""
    offset = 0
    length = len(data)
    while offset < length:
        i = 0
        old_offset = offset
        items = []
        while offset < length and i < items_per_line:
            items.append(data[offset])
            offset += 1
            i += 1
        ascii_rep = "".join(map(lambda n: chr(n) if n >= 32 and n < 128 else '.', items))

        # if there are to few items, fill the gap to ensure clean output
        line = " ".join(map(lambda n: "%02x" % n, items))
        line += " " + " ".join(["**"] * (items_per_line - len(items)))
        print("$%06x: %s    %s" % (old_offset, line, ascii_rep))


def print_hunks(hunks, disassembled):
    for i, hunk in enumerate(hunks):
        block = hunk[0]
        if block[0] == 'NAME':
            print("%d: '%s' -> '%s'" % (i, block[0], block[1]))
        elif block[0] == 'BSS':
            print("%d: '%s' -> %d" % (i, block[0], block[1]))
        elif block[0] == 'CODE':
            print("%d: '%s', size = %d" % (i, block[0], len(block[1])))
            code = block[1]
            if disassembled:
                disassemble(code)
            else:
                print_data(code)
        elif block[0] == 'DATA':
            print("%d: '%s', size = %d" % (i, block[0], len(block[1])))
            print_data(block[1])
        else:
            print("%d: '%s'" % (i, block[0]))
        print("")


def dump_code_hunks(hunkfile, hunks):
    for i, hunk in enumerate(hunks):
        block = hunk[0]
        if block[0] == 'CODE':
            with open("%s-%02d" % (hunkfile, i), 'wb') as outfile:
                outfile.write(block[1])


def print_overview(hunks):
    for i, hunk in enumerate(hunks):
        print(" %d: %s" % (i, hunk[0][0]))
        for block in hunk[1:]:
            print("    %s" % block[0])
        print("    END")


def parse_hunkfile(hunkfile, disassembled, dump_code, detail):
    """Top level parsing function"""
    with open(hunkfile, 'rb') as infile:
        id = infile.read(4)

        # can be Header or Unit
        is_loadfile = True
        if id == HUNK_BLOCK_HEADER:
            print("Hunk Header (03f3)")
            library_names = read_string_list(infile)
            print("\tLibraries: ", library_names)
            hunk_table_size = read_int32(infile)
            first_slot = read_int32(infile)
            last_slot = read_int32(infile)
            print("\t%d hunks (%d-%d)\n" % (hunk_table_size, first_slot, last_slot))
            num_hunk_sizes = last_slot - first_slot + 1
            """
            print("first slot: %d, last slot: %d, # hunk sizes: %d" % (first_slot,
                                                                       last_slot,
                                                                       num_hunk_sizes))
            """
            hunk_sizes = [read_int32(infile) for i in range(num_hunk_sizes)]
            #print("hunk sizes: ", hunk_sizes)
        elif id == HUNK_BLOCK_UNIT:
            strlen = read_int32(infile) * 4
            #print("# name: %d" % strlen)
            unit_name = str(infile.read(strlen))
            print("Hunk unit (03e7)")
            print("\tName: %s\n" % unit_name)
        else:
            is_loadfile = False
            raise Exception('Unsupported header type')

        blocks = read_blocks(infile, is_loadfile)
        hunks = group_blocks(blocks)

        if disassembled or detail:
            print_hunks(hunks, disassembled)
        else:
            print_overview(hunks)

        if dump_code:
            dump_code_hunks(hunkfile, hunks)

if __name__ == '__main__':
    description = """dalf.py - Dumps Amiga Load Files with Python (c) 2014 Wei-ju Wu"""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('hunkfile', help="hunk format file")
    parser.add_argument('--disassemble', action="store_true", default=False,
                        help="show disassembly")
    parser.add_argument('--detail', action="store_true", default=False,
                        help="show data")
    parser.add_argument('--dump_code', action="store_true", default=False,
                        help="extracts and dumps each code hunk to a separate file")

    args = parser.parse_args()
    parse_hunkfile(args.hunkfile, disassembled=args.disassemble, dump_code=args.dump_code,
                   detail=args.detail)
