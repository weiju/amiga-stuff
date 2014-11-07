#!/usr/bin/env python3
"""A tool like dalf.rexx that can be used to inspect the structure of
an Amiga Hunk file.
"""
import argparse
import struct

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


def read_int16(infile):
    return struct.unpack('>h', infile.read(2))[0]


def read_string_list(infile):
    result = []
    strlen = read_int32(infile)
    while strlen > 0:
        result.append(str(infile.read(strlen)))
        strlen = read_int32(infile)
        
    return []


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

def parse_hunkfile(hunkfile):
    """Top level parsing function"""
    with open(hunkfile, 'rb') as infile:
        id = infile.read(4)

        # can be Header or Unit
        is_loadfile = True
        if id == HUNK_BLOCK_HEADER:
            library_names = read_string_list(infile)
            print("Libraries: ", library_names)
            hunk_table_size = read_int32(infile)
            print("size hunk table: %d" % hunk_table_size)
            first_slot = read_int32(infile)
            last_slot = read_int32(infile)
            num_hunk_sizes = last_slot - first_slot + 1
            print("first slot: %d, last slot: %d, # hunk sizes: %d" % (first_slot,
                                                                       last_slot,
                                                                       num_hunk_sizes))
            hunk_sizes = [read_int32(infile) for i in range(num_hunk_sizes)]
            print("hunk sizes: ", hunk_sizes)
        elif id == HUNK_BLOCK_UNIT:
            strlen = read_int32(infile) * 4
            print("# name: %d" % strlen)
            unit_name = str(infile.read(strlen))
            print("UNIT '%s'" % unit_name)
        else:
            is_loadfile = False
            raise Exception('Unsupported header type')
        
        blocks = read_blocks(infile, is_loadfile)
        print("%d blocks read: " % len(blocks))
        for i, block in enumerate(blocks):
            if block[0] == 'NAME':
                print("%d: '%s' -> '%s'" % (i, block[0], block[1]))
            elif block[0] == 'BSS':
                print("%d: '%s' -> %d" % (i, block[0], block[1]))
            else:
                print("%d: '%s'" % (i, block[0]))


if __name__ == '__main__':
    description = """hunk - hunk file reader (c) 2014 Wei-ju Wu"""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('hunkfile', help="hunk format file")
    args = parser.parse_args()
    parse_hunkfile(args.hunkfile)
