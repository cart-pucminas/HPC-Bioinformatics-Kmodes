#!/usr/bin/env python

import sys

def convert_k_modes_input(filename):

    input = open(filename, 'r')
    output = open(filename + '.dat', 'w')
    for line in input:
    
        property1 = "0001" if line[160:161] == '1'  else "0010"
        property2 = "0001" if line[161:162] == '1'  else "0010"
        property3 = "0001" if line[162:163] == '1'  else "0010"

        #print(line)
        #print(line[160:161] + " " + line[161:162] + " " +line[162:] )
        #print(property1 + " " + property2 + " " +property3 )
        
        newLine = line[:160]
        output.write(newLine + property1 + property2 + property3 + "\n")
    input.close
    output.close;
	


BASES = [
        #"../input_mpi/gallus/", 
        #"../input_mpi/Rattusnovergicus/",
        # "../input_mpi/Rattusnovergicus/", 
         "../input_mpi/Musmusculus/", 
        # "../input_mpi/celegans/", 
        # "../input_mpi/athaliana/", 
        # "../input_mpi/Drosophila/", 
        # "../input_mpi/HomoSapiens/"
        ]
		 
		 
for base in BASES:
    print "Converting base: " + base
    for i in range (1,11):
        convert_k_modes_input(base + "cluster" + str(i))

