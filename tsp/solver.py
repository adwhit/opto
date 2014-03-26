#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from cffi import FFI

VERBOSE = False

def solve_it(input_data):
    lines = input_data.split("\n")
    nnodes = int(lines[0].strip())
    arr = [(float(n[0]), float(n[1])) for n in [line.split() for line in lines[1:] if line]]
    xs, ys = zip(*arr)

    ffi = FFI()

    ffi.cdef("""
    typedef struct {
        short *bytes;
        double score;
    } Gene;

    void pyffi(int nnodes, double *xs, double *ys, int verbose);
    """)

    if VERBOSE:
        bit = 1
    else:
        bit = 0

    lib = ffi.dlopen("libgene.so")
    lib.pyffi(nnodes, xs, ys, bit)

if __name__ == '__main__':
    if len(sys.argv) > 2:
        if sys.argv[2] == "-v":
            VERBOSE = True
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        input_data_file = open(file_location, 'r')
        input_data = ''.join(input_data_file.readlines())
        input_data_file.close()
        print solve_it(input_data)
    else:
        print 'This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/gc_4_1)'

