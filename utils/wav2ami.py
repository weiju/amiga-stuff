#!/usr/bin/env python
import argparse
import wave
import struct

def conv_amplitude(amp_bstr):
    """converts 16 bit amplitudes into 8 bit amplitudes"""
    return struct.unpack(">H", amp_bstr)[0] >> 8


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="wav2ami 1.0")
    parser.add_argument('wavfile')

    args = parser.parse_args()
    print args.wavfile
    infile = wave.open(args.wavfile)
    print "# channels: ", infile.getnchannels()
    print "sample width: ", infile.getsampwidth()
    print "frame rate: ", infile.getframerate()
    print "# frames: ", infile.getnframes()
    num_frames = infile.getnframes()
    data = [conv_amplitude(infile.readframes(1)) for _ in range(num_frames)]
    #print data
    infile.close()
