#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

import sys
from math import modf
from datetime import timedelta

def somar(valores):
	soma = 0
	for v in valores:
		soma += v
	return soma


def media(valores):
	soma = somar(valores)
	qtd_elementos = len(valores)
	media = soma / float(qtd_elementos)
	return media


def variancia(valores):
	_media = media(valores)
	soma = 0
	_variancia = 0

	for valor in valores:
		soma += math.pow( (valor - _media), 2)
	_variancia = soma / float( len(valores) )
	return _variancia


def desvio_padrao(valores):
	return math.sqrt( variancia(valores) )

for i in range(1,n+1):
    file = open( 'results' + str(i), 'r')
    contador = 0
    ac = []
    pr = []
    se = []
    es = []
    acx = []
    for (line in file):
        index  = line.find(': ');
		if contador == 4:
        	ac.append(float(line[index:]))
		if contador == 5:
        	pr.append(float(line[index:]))
		if contador == 6:
        	se.append(float(line[index:]))
		if contador == 7:
        	es.append(float(line[index:]))        	        	        	
		if contador == 8:
        	acx.append(float(line[index:]))        	        	        	        	
        contador+=1

print 'ac' + str(variancia(ac))
print 'pr' + str(variancia(pr))
print 'se' + str(variancia(se))
print 'es' + str(variancia(es))
print 'acx' + str(variancia(acx))



                 
 
