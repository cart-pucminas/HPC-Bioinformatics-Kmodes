#!/usr/bin/env python
# This Python file uses the following encoding: utf-8

import sys
from math import modf
from datetime import timedelta

def evalResults(workingDir,n):


    accuracyFinal = 0.0
    precisionFinal = 0.0
    sensibilityFinal = 0.0
    especificityFinal = 0.0
    adjustedAccuracyFinal = 0.0
    timeMinutes = 0.0
    timeSeconds = 0.0
    timeMilliseconds = 0.0

    for i in range(1,n+1):
        
        countPos = 0
        countNeg = 0
        truePos = 0
        trueNeg = 0
        falsePos = 0
        falseNeg = 0
        
        precision = 0.0
        sensibility = 0.0
        especificity = 0.0
        

        log = open(workingDir + 'log' + str(i) + '.txt', 'r')
        
        for line in log:
        	if 'real' in line:
        		index = line.find('real\t')+5
        		finalIndex = line.find('m')
        		minutes = line[index:finalIndex]
        		timeMinutes+=float(minutes)
        		
        		index = finalIndex + 1 
        		finalIndex = line.find('.')
        		seconds = line[index:finalIndex]
        		timeSeconds+=float(seconds)
        		
        		index = finalIndex +1
        		finalIndex = line.find('s')
        		hundredths = line[index:finalIndex]
        		timeMilliseconds+=float(hundredths)
        		
        log.close()        
  
        input = open(workingDir + 'test' + str(i), 'r')        
        for line in input:
            if line[0] == '+':
                countPos+=1
            else:
                countNeg+=1
        input.close()
                    
        file = open(workingDir + 'predictions' + str(i), 'r')
        for j in range(countPos):
            line = file.readline()
            if line[0] == '-':
                falseNeg+= 1
            else:
                truePos+=1
        for j in range(countPos+1,countNeg + countPos):
            line = file.readline()
            if line[0] == '-':
                trueNeg+= 1
            else:
                falsePos+=1
        file.close()

        accuracy = 100*(float(truePos+trueNeg)/(truePos+trueNeg+falsePos+falseNeg))
        if truePos != 0:
            precision = 100*(float(truePos)/(truePos+falsePos))
            sensibility = 100*(float(truePos)/(truePos+falseNeg));
        if trueNeg != 0:
            especificity = 100*(float(trueNeg)/(trueNeg+falsePos))

        adjustedAccuracy = float(sensibility+especificity)/2;
        
        print 'Creating file ' + workingDir + 'results' + str(i)
        output = open(workingDir + 'results' + str(i), 'w')
        
        output.write('Verdadeiro Positivos: ' + str(truePos) + '\n')
        output.write('Verdadeiro Negativos: ' + str(trueNeg) + '\n')
        output.write('Falso Positivos: ' + str(falsePos) + '\n')
        output.write('Falso Negativos: ' + str(falseNeg) + '\n')
        output.write('Acurácia: ' + str(accuracy) + '\n')
        output.write('Precisão: ' + str(precision) + '\n')
        output.write('Sensibilidade: ' + str(sensibility) + '\n')
        output.write('Especificidade: ' + str(especificity) + '\n')
        output.write('Acurácia Ajustada: ' + str(adjustedAccuracy) + '\n')

        output.close

        accuracyFinal+=accuracy;
        precisionFinal+=precision;
        sensibilityFinal+=sensibility;
        especificityFinal+=especificity;
        adjustedAccuracyFinal+=adjustedAccuracy;

    accuracyFinal=(accuracyFinal/n);
    precisionFinal=(precisionFinal/n);
    sensibilityFinal=(sensibilityFinal/n);
    especificityFinal=(especificityFinal/n);
    adjustedAccuracyFinal=(adjustedAccuracyFinal/n);
    
    timeHundredths=timeMilliseconds/n
    timeSeconds=timeSeconds/n
    timeMinutes=timeMinutes/n 
    timeFormatted = timedelta(0,timeSeconds,0,timeMilliseconds,timeMinutes)

    output = open(workingDir + 'resultsFinal', 'w')
    
    output.write('Acurácia: ' + str(accuracyFinal) + '\n')
    output.write('Precisão: ' + str(precisionFinal) + '\n')
    output.write('Sensibilidade: ' + str(sensibilityFinal) + '\n')
    output.write('Especificidade: ' + str(especificityFinal) + '\n')
    output.write('Acurácia Ajustada: ' + str(adjustedAccuracyFinal) + '\n')
    output.write('Tempo Médio: ' + str(timeFormatted) + '\n')    
    output.write('Tempo Médio em segundos: ' + str(timeFormatted.total_seconds()) + '\n')        
        
    output.close


