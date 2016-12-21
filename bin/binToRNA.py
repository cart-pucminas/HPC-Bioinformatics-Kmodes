#!/usr/bin/env python

import sys

input = open(sys.argv[1], 'r')
output = open(sys.argv[2], 'w')
for line in input:
    seq = ''
    for i in range(0,157,4):
        c = line[i:i+4];
        if c == '0001':
            seq += 'a'
        elif c == '0010':
            seq += 'c'
        elif c == '0100':
            seq += 'g'
        elif c == '1000':
            seq += 't'
    for i in range (160,len(line)):
        seq += line[i]
    output.write(seq)
output.close()