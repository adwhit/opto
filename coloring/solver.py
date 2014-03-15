#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from cffi import FFI

def solve_it(input_data):
    arr = [(int(n[0]), int(n[1])) for n in [line.split() for line in input_data.split("\n") if line]]
    nnodes = arr[0][0]
    nedges = arr[0][1]
    n1arr, n2arr = zip(*arr[1:])

    ffi = FFI()

    ffi.cdef("""
    struct Result {
        int ncolours;
        int nnodes;
        int *colours;
    };

    struct Result solve(int *n1arr, int *n2arr, int nnodes, int nedges);
    """)

    lib = ffi.dlopen("libgraph.so")
    res = lib.solve(n1arr, n2arr, nnodes, nedges)

    out = "%d 0\n" % res.ncolours
    out += " ".join([str(res.colours[i]) for i in range(res.nnodes)])
    return out

if __name__ == '__main__':
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        input_data_file = open(file_location, 'r')
        input_data = ''.join(input_data_file.readlines())
        input_data_file.close()
        print solve_it(input_data)
    else:
        print 'This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/gc_4_1)'

