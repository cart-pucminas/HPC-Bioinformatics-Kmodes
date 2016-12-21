#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

import sys
import os
import fileHandle
import platform
import resultHandle


#PROFILER='/usr/local/cuda-7.5/bin/nvprof --system-profiling on --devices 0 '
PROFILER = ''
workingDir = sys.argv[4]
for i in range (1,11):
    commandLine = '(time ' + PROFILER + './kmodes-openmp ' + \
                   workingDir + 'cluster' + str(i) + \
                   ' ' + sys.argv[2] + \
                   ') &> ' + sys.argv[4] +  'log' + str(i) + '.txt'
    print commandLine
    os.system(commandLine)

#resultHandle.evalResults(workingDir,10)
