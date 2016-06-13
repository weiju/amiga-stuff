#!/usr/bin/env python3
"""
Convert Cloanto ROM to normal Kickstart ROM
"""
import argparse

CLOANTO_ID = b'AMIROMTYPE1'
SIZE_BOOTROM = 0x2000
SIZE_590     = 0x4000
SIZE_4091    = 0x8000
SIZE_CLASSIC = 0x40000
SIZE_MODERN  = 0x80000

ROM_SIZES = {SIZE_BOOTROM, SIZE_590, SIZE_4091, SIZE_CLASSIC, SIZE_MODERN}

def decode_cloanto(rom, key):
    """Decodes an encoded Kickstart ROM and returns a decoded ROM"""
    rom = bytearray(rom)
    key = bytearray(key)
    result = bytearray(len(rom))
    key_size = len(key)
    key_index = 0

    for i in range(len(rom)):
        result[i] = rom[i] ^ key[key_index]
        key_index = (key_index + 1) % key_size

    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Cloanto Decoder 1.0")
    parser.add_argument('romfile')
    parser.add_argument('keyfile')
    parser.add_argument('target')
    args = parser.parse_args()

    with open(args.keyfile, 'rb') as infile:
        key_data = infile.read()

    with open(args.romfile, 'rb') as infile:
        file_id = infile.read(len(CLOANTO_ID))
        if file_id == CLOANTO_ID:
            data = infile.read()
            size = len(data)
            if size not in ROM_SIZES:
                raise Exception("invalid rom size")
            decoded = decode_cloanto(data, key_data)
            with open(args.target, 'wb') as outfile:
                outfile.write(bytes(decoded))
        else:
            raise Exception('not a cloanto rom: ', file_id)
