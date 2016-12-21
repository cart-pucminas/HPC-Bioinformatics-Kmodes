#!/usr/bin/env python

import random
import sys

for i in range(int(sys.argv[1])):
	s = ''
	
	for j in range(40):
		x = int(random.random()*4)
		
		if x == 0: s+='0001'
		elif x == 1: s+= '0010'
		elif x == 2: s+= '0100'
		else: s+= '1000'
	
	for j in range(3):
		s += str(int(random.random()*2))
		
	print s
		
