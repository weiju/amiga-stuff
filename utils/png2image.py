#!/usr/bin/env python3

"""
For license, see gpl-3.0.txt
"""
from PIL import Image
import argparse
import math
import sys
import os

def chunks(l, n):
    for i in range(0, len(list(l)), n):
        yield l[i:i+n]

def color_to_plane_bits(color, depth):
    """returns the bits for a given pixel in a list, lowest to highest plane"""
    result = [0] * depth
    for bit in range(depth):
        if color & (1 << bit) != 0:
            result[bit] = 1
    return result


def extract_planes(im, depth):
    imdata = im.getdata()
    width, height = im.size

    map_words_per_row = int(width / 16)
    if width % 16 > 0:
        map_words_per_row += 1

    # create the converted planar data
    planes = [[0] * (map_words_per_row * height) for _ in range(depth)]
    for y in range(height):
        x = 0
        while x < width:
            # build a word for each plane
            for i in range(min(16, width - x)):
                # get the palette index for pixel (x + i, y)
                color = imdata[y * width + x + i]  # color index
                planebits = color_to_plane_bits(color, depth)
                # now we need to "or" the bits into the words in their respective planes
                wordidx = (x + i) / 16  # word number in current row
                pos = int(y * map_words_per_row + wordidx)  # list index in the plane
                for planeidx in range(depth):
                    if planebits[planeidx]:
                        planes[planeidx][pos] |= (1 << (15 - (x + i) % 16)) # 1 << ((x + i) % 16)
            x += 16
    return planes, map_words_per_row


def interleave_planes(planes, map_words_per_row):
    """transforms a set of bitplanes into a large array of 16-bit
    word rows. each representing a line of an image
    """
    # 1. for each plane generate a list of lists of <map_words_per_row>
    # values
    chunked = list([list(chunks(plane, map_words_per_row)) for plane in planes])

    # 2. for each plane, add the rows one after another to the result list
    num_rows = len(chunked[0])
    result = []
    for i in range(num_rows):
        for chunk in chunked:
            result.append(chunk[i])
    return result

def write_amiga_image(im, outfile, img_name, use_intuition, interleaved, verbose):
    imdata = im.getdata()
    width, height = im.size
    if width % 16 != 0:
        raise Exception("width needs to be a multiple of 16, is: %d" % width)

    # colors is a list of 3-integer lists ([[r1, g1, b1], ...])
    colors = [i for i in chunks([b for b in im.palette.tobytes()], 3)]
    depth = round(math.log(len(colors), 2))
    # fill the missing colors with black entries
    num_missing_colors = 2 ** depth - len(colors)
    colors += [[0, 0, 0]] * num_missing_colors

    planes, map_words_per_row = extract_planes(im, depth)

    if verbose:
        if interleaved:
            print('using interleaved mode')
        print("image width: %d height: %d depth: %d" % (width, height, depth))
    if use_intuition:
        outfile.write('#include <intuition/intuition.h>\n\n')
    imgdata_varname = '%s_data' % img_name
    outfile.write("/* Ensure that this data is within chip memory or you'll see nothing !!! */\n");
    outfile.write('UWORD __chip %s[] = {\n' % imgdata_varname)
    indent = 4

    if interleaved:
        interleaved_data = interleave_planes(planes, map_words_per_row)
        lines = [(' ' * indent + ','.join(['0x%04x' % v for v in row]))
                 for row in interleaved_data]
        outfile.write(',\n'.join(lines))
    else:
        for i, plane in enumerate(planes):
            if i > 0:
                outfile.write(',\n')
            outfile.write(' ' * indent + '// plane %d\n' % i)
            lines = [(' ' * indent) + ','.join(['0x%04x' % i for i in chunk])
                     for chunk in chunks(plane, map_words_per_row)]
            outfile.write(',\n'.join(lines))
    outfile.write('\n};\n\n')

    planepick = 2 ** depth - 1  # currently we always use all of the planes
    if use_intuition:
        outfile.write('struct Image %s = {\n' % img_name)
        outfile.write(' ' * indent + '0, 0, %d, %d, %d, %s,\n' % (width, height, depth, imgdata_varname));
        outfile.write(' ' * indent + '%d, 0, NULL\n' % planepick)  # PlanePick, PlaneOnOff
        outfile.write('};\n\n')

    outfile.write('UWORD %s_colors[%d] = {\n' % (img_name, len(colors)))
    colors4 = []
    for i, color in enumerate(colors):
        r, g, b = color
        r4 = r >> 4
        g4 = g >> 4
        b4 = b >> 4
        colors4.append('0x%x%x%x' % (r4, g4, b4))
    outfile.write((' ' * indent) + ','.join(colors4) + '\n')
    outfile.write('};\n\n')

DESCRIPTION = """png2image - Amiga Image Converter

This tool converts a PNG image to a C header file containing image
information in planar format. Optionally it generates the data
structures needed for usage as Intuition image."""
if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description=DESCRIPTION)
    parser.add_argument('pngfile', help="input PNG file")
    parser.add_argument('headerfile', help="output header file")
    parser.add_argument('--img_name', default='image', help="variable name of the image")
    parser.add_argument('--use_intuition', action='store_true', help="generate data for Intuition")
    parser.add_argument('--verbose', action='store_true', help="verbose mode")
    parser.add_argument('--interleaved', action='store_true', help="store data in interleaved manner")

    args = parser.parse_args()
    im = Image.open(args.pngfile)
    if not os.path.exists(args.headerfile):
        with open(args.headerfile, 'w') as outfile:
            write_amiga_image(im, outfile, img_name=args.img_name,
                              use_intuition=args.use_intuition,
                              interleaved=args.interleaved,
                              verbose=args.verbose)
    else:
        print("file '%s' already exists." % args.headerfile)
