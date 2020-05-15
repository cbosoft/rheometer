#!/usr/bin/local/python3.7
'''docstring'''

import sys

import numpy as np

assert len(sys.argv) > 2


def read_csv(path):
    '''docstring'''
    print(f"Reading {path}")
    with open(path) as csvf:
        data = list(zip(*[[float(f) for f in line.split(',')] for line in csvf.readlines()]))
    return data

for path in sys.argv[1:]:
    data = read_csv(path)
    std = np.std(data[-1])
    print(std)
