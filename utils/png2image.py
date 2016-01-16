#!/usr/bin/python

"""
For license, see gpl-3.0.txt
"""
from PIL import Image
import argparse
import math
import sys

def chunks(l, n):
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def color_to_plane_bits(color, depth):
    """returns the bits for a given pixel in a list, lowest to highest plane"""
    result = [0] * depth
    for bit in range(depth):
        if color & (1 << bit) != 0:
            result[bit] = 1
    return result

def write_amiga_image(image, outfile):
    imdata = im.getdata()
    width, height = im.size
    colors = [i for i in chunks(map(ord, im.palette.tobytes()), 3)]
    depth = int(math.log(len(colors), 2))

    map_words_per_row = width / 16
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
                pos = y * map_words_per_row + wordidx  # list index in the plane
                for planeidx in range(depth):
                    if planebits[planeidx]:
                        planes[planeidx][pos] |= (1 << (15 - (x + i) % 16)) # 1 << ((x + i) % 16)
            x += 16

    outfile.write('#include <intuition/intuition.h>\n\n')

    outfile.write("/* Ensure that this data is within chip memory or you'll see nothing !!! */\n");
    outfile.write('UWORD imdata[] = {\n')
    for plane in planes:
        for chunk in chunks(plane, map_words_per_row):
            outfile.write(','.join(map(str, chunk)))
            outfile.write(',\n')
        outfile.write('\n')
    outfile.write('};\n\n')


    planepick = 2 ** depth - 1  # currently we always use all of the planes
    outfile.write('struct Image image = {\n')
    outfile.write('  0, 0, %d, %d, %d, imdata,\n' % (width, height, depth));
    outfile.write('  %d, 0, NULL\n' % planepick)  # PlanePick, PlaneOnOff
    outfile.write('};\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Amiga Image converter')
    parser.add_argument('pngfile')

    args = parser.parse_args()
    im = Image.open(args.pngfile)
    write_amiga_image(im, sys.stdout)
