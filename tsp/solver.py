#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import viewer
from cffi import FFI

VERBOSE = False
GRAPH = False
PREVIEW = False
SNAPSHOTS = False

def solve_it(input_data):
    lines = input_data.split("\n")
    nnodes = int(lines[0].strip())
    arr = [(float(n[0]), float(n[1])) for n in [line.split() for line in lines[1:] if line]]
    xs, ys = zip(*arr)

    if PREVIEW:
        g = viewer.creategraph(nnodes)
        viewer.draw(g, arr)
        sys.exit()

    ffi = FFI()

    ffi.cdef("""
    typedef struct {
        short *bytes;
        double score;
    } Gene;

    int *pyffi(int nnodes, double *xs, double *ys, int verbose, int snapshots);
    """)

    if VERBOSE:
        bit = 1
    else:
        bit = 0

    if SNAPSHOTS:
        snp = 1
    else:
        snp = 0

    lib = ffi.dlopen("libgene.so")
    route = lib.pyffi(nnodes, xs, ys, bit, snp)

    route = list(route[0:nnodes])

    if GRAPH:
        g = viewer.creategraph(nnodes, route)
        viewer.draw(g, arr)

    return route

if __name__ == '__main__':
    if "-v" in sys.argv:
        VERBOSE = True
    if "-g" in sys.argv:
        GRAPH = True
    if "-p" in sys.argv:
        PREVIEW = True
    if "-s" in sys.argv:
        SNAPSHOTS = True

    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        input_data_file = open(file_location, 'r')
        input_data = ''.join(input_data_file.readlines())
        input_data_file.close()
        result = solve_it(input_data)
        if not SNAPSHOTS :
            result
    else:
        print 'This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/gc_4_1)'

