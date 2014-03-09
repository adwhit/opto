#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
from subprocess import Popen, PIPE

def solve_it(input_data):

    tmp_file_name = 'tmp.data'
    tmp_file = open(tmp_file_name, 'w')
    tmp_file.write(input_data)
    tmp_file.close()

    process = Popen(['./knap', tmp_file_name], stdout=PIPE)
    (stdout, stderr) = process.communicate()

    os.remove(tmp_file_name)
    return stdout.strip()



if __name__ == '__main__':
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        input_data_file = open(file_location, 'r')
        input_data = ''.join(input_data_file.readlines())
        input_data_file.close()
        print solve_it(input_data)
    else:
        print 'This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/ks_4_0)'
