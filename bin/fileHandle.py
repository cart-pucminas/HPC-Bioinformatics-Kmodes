#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

import sys
import random
import os
import array

# Avoid to create files that already exists
def safeOpen(filename):
    if not os.path.exists(filename):
         print 'Creating file ' + filename
         return open(filename,'w')
    else:
        print 'File ' + filename + ' already exist, skiping'
        return None

def binaryStrToSvmFormat(line,isPostive):
    newStr = ''
    if (isPostive):
        newStr = '+1 '
    else:
        newStr = '-1 '
    for idx,c in enumerate(line):
        if c == '1':
            newStr += str(idx+1) + ':1 '
    newStr += '\n'
    return newStr

def convertToBinarySequence(filename):
    input = open(filename, 'r')
    output = safeOpen(filename + '.binseq')
    numberOfLines = 0
    sequence = []
    
    for line in input:
        seq = ''
        numberOfLines+=1
        for c in line:
            if c == 'a':
                seq += '0001'
            elif c == 'c':
                seq += '0010'
            elif c == 'g':
                seq += '0100'
            elif c == 't':
                seq += '1000'
            elif c == '0' or c == '1':
                seq += c # case is a property flag
        seq+= '\n'
        sequence.append(seq);
        if (output is not None):
            output.write(seq)
    
    input.close()
    if (output is not None):
        output.close()
    
    return sequence

def createTestsFiles (sequence,filePrefix,div):
    n = len(sequence)
    for i in range (1,div+1):
        if (i == div):
            count = len(sequence)
        else:
            count = n / div
        
        output = safeOpen(filePrefix + str(i))
        if (output is None):
            count = 0
        while (count > 0):
            index = random.randrange(len(sequence))
            output.write(sequence.pop(index))
            count-=1
        if (output is not None):
            output.close

# Join testPosI with TestNegI
# and convert the file for svm format
def createFinalTestsFiles(workingDir,div):
    postiveCount=array.array('i',(0 for i in range(0,div)))
    for i in range (1,div+1):
        output = open(workingDir + 'test' + str(i),'w')
        input = open(workingDir + 'testPos' + str(i), 'r')
        print 'Opening file ' + workingDir + 'testPos' + str(i)
        for line in input:
            newline = binaryStrToSvmFormat(line,True)
            output.write(newline)
            postiveCount[i-1]+=1
        input.close()
        intput2 = open(workingDir + 'testNeg' +str(i))
        for line in intput2:
            newline = binaryStrToSvmFormat(line,False)
            output.write(newline)
        intput2.close()
        output.close()
    return postiveCount

# create files for cluster
def createClustersFiles (workingDir,div):
    for i in range (1,div+1):
        output = safeOpen(workingDir + 'cluster' + str(i))
        if (output is None):
            continue
        for j in range (1,div+1):
            if i != j:
                input = open(workingDir + 'testNeg' + str(j))
                for line in input:
                    # add space for each 64 char, for help
                    # parse binary sequence to 64bit integer
                    newline = line[:64] + ' ' + line[64:128] + ' ' + line[128:160]
                    for j in range (160,len(line)):
                        if (line[j] == '1'):
                            newline+='0001'
                        elif line[j] == '0':
                            newline+='0010'
                    output.write(newline + '\n')
                input.close
        output.close

def createTrainFiles (workingDir,div):
    for i in range (1,div+1):
        output = safeOpen(workingDir + 'train' + str(i))
        if (output is None):
            continue
        for j in range (1,div+1):
            if i != j:
                input = open(workingDir + 'testPos' + str(j))
                for line in input:
                    newline = binaryStrToSvmFormat(line,True)
                    output.write(newline)
                input.close
        negativos = convertToBinarySequence(workingDir + 'cluster' + str(i) + '.out')
        for line in negativos:
            newline = binaryStrToSvmFormat(line,False)
            output.write(newline)
        output.close




