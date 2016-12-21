#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

import sys
import os
import fileHandle
import platform
import resultHandle

workingDir = sys.argv[1]

folds = 10

# read and create files (if not exists)
# return an array with all lines

positivos = fileHandle.convertToBinarySequence(workingDir + 'positivos')
negativos = fileHandle.convertToBinarySequence(workingDir + 'negativos')

totalPositive = len(positivos)

fileHandle.createTestsFiles(positivos,workingDir + 'testPos',folds)
fileHandle.createTestsFiles(negativos,workingDir + 'testNeg',folds)


postiveCount = fileHandle.createFinalTestsFiles(workingDir,folds)

fileHandle.createClustersFiles(workingDir,folds)

for i in range (1,folds+1):
    commandLine = '(time ' + './mpi-means ' + \
                workingDir + 'cluster' + str(i) + \
                ' ' + str(totalPositive - postiveCount[i-1]) + \
                ') &> ' + workingDir +  'log' + str(i) + '.txt'
    print commandLine
    os.system(commandLine)

fileHandle.createTrainFiles(workingDir,folds)

svm_learn = './svm_learn'
svm_classify = './svm_classify'
if platform.system() == 'Darwin':
    svm_learn = './svm_learn_osx'
    svm_classify = './svm_classify_osx'

for i in range (1,folds+1):
    commandLine = svm_learn + ' -t 1 -d 4 -c 0.5 ' + \
               workingDir + 'train' + str(i) + \
               ' ' + workingDir + 'model' + str(i)
    print commandLine
    os.system(commandLine)

for i in range (1,folds+1):
    commandLine = svm_classify  + \
               ' ' + workingDir + 'test' + str(i) + \
               ' ' + workingDir + 'model' + str(i) + \
               ' ' + workingDir + 'predictions' + str(i)
    print commandLine
    os.system(commandLine)

resultHandle.evalResults(workingDir,folds)

