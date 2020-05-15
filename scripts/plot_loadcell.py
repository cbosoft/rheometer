#!/usr/bin/local/python3.7
'''docstring'''

import sys

from matplotlib import pyplot as plt

assert len(sys.argv) > 1


def read_csv(path):
    '''docstring'''
    print(f"Reading{path}")
    with open(path) as csvf:
        data = list(zip(*[[float(f) for f in line.split(',')] for line in csvf.readlines()]))
    return data

log_data = read_csv(sys.argv[1])


plt.plot(log_data[0], log_data[-1])
plt.show()
