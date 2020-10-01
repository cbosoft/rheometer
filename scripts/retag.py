#!/usr/bin/env python

import sys
import os
import re


def reglob(pattern):
    files = os.listdir()
    regex = re.compile(pattern)
    return [f for f in files if regex.match(f)]


def retag(files, newtag):
    result = list()
    for f in files:
        basename = os.path.basename(f)
        dirname = os.path.dirname(f)
        parts = basename.split('_')
        ext = '.'.join(basename.split('.')[1:])
        parts[-1] = newtag + '.' + ext
        basename = '_'.join(parts)
        f = os.path.join(dirname, basename)
        result.append(f)
    return result

def main(args):

    patterns = list()
    files = list()
    newtag = None
    dummy = False

    i = 0
    while i < len(args): 
        key = args[i]
        value = args[i+1]
        if key[0] == '-':
            if key in ('--pattern', '-p'):
                patterns.append(value)
                i += 1
            elif key in ('--tag', '-t'):
                newtag = value
                i += 1
            elif key in ('--dummy', '-d'):
                dummy = True
            else:
                print(f'unknown key {key}')
                exit(1)
        else:
            files.append(key)
        i += 1
    
    if not newtag:
        print('what do you want to change the tag to?')
        exit(1)

    newfiles = retag(files, newtag)

    for f, nf in zip(files, newfiles):
        cmd = f'mv "{f}" "{nf}"'
        print(cmd)
        if not dummy:
            os.system(cmd)


if __name__ == '__main__':
    main(sys.argv[1:])

