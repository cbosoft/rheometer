#!/usr/bin/local/python3.7
'''docstring'''

import sys

from matplotlib import pyplot as plt
import numpy as np

if len(sys.argv) != 2:
    print("Usage: plot_adc.py <csv>")
    exit(1)



def read_csv(path):
    '''docstring'''
    print(f"Reading{path}")
    with open(path) as csvf:
        data = list(zip(*[[float(f) for f in line.split(',')] for line in csvf.readlines()]))
    return data


log_data = read_csv(sys.argv[1])
time = log_data[0]
adc = np.subtract(log_data[2], log_data[3])
plt.plot(time, adc)
plt.show()
