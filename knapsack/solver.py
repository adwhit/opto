#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from cffi import FFI

def solve_it(input_data):
    arr = [(int(n[0]), int(n[1])) for n in [line.split() for line in input_data.split("\n") if line]]
    nitems = arr[0][0]
    capacity = arr[0][1]
    values, weights = zip(*arr[1:])

    ffi = FFI()

    ffi.cdef("""
    typedef enum {false, true} bool;

    typedef struct {
        bool success;
        int value;
        bool *route;
    } Result;

    Result run(int *values, int *weights, int nitems, int capacity);
    """)

    fpath = "data/ks_4_0"

    lib = ffi.dlopen("libknap.so")
    res = lib.run(values, weights, nitems, capacity)

    out = "%d 1\n" % res.value
    out += " ".join([str(res.route[i]) for i in range(nitems)])

    return out



if __name__ == '__main__':
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        input_data_file = open(file_location, 'r')
        input_data = ''.join(input_data_file.readlines())
        input_data_file.close()
        print solve_it(input_data)
    else:
        print 'This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/ks_4_0)'

