import time
import sys
sys.path.append('..\modules')
from common import common
from seawat import seawat
from hydrologicalSimulation import hydrologicalSimulation
from edaMoC import edaMoC
import numpy
import os
from ctypes import *
import ctypes
from numpy.ctypeslib import ndpointer
import math
import matplotlib.pyplot as plt
from matplotlib.pyplot import *
import random
MoCFuncType = CFUNCTYPE(c_int, c_double, POINTER(c_double), POINTER(c_double) )
exePath   = os.path.join(os.getcwd(), 'models', 'SEAWAT', 'bin','swt_v4.exe')

commonFunctions = common()

def loadSystemSimulator():

	fn_lib = os.path.join(os.getcwd(), '..\Release', 'HydrologicalModLib.dll')
	system = CDLL(fn_lib)
	system.startSim.argtypes      = [c_double]
	system.startSim.restype       = ctypes.c_int
	
	system.createSignal.argtypes  = [POINTER(c_char)]
	system.createSignal.restype   = ctypes.c_int
	
	system.createTDFMoC.argtypes  = [POINTER(c_char), c_double, MoCFuncType]
	system.createTDFMoC.restype   = ctypes.c_int
	
	system.createLSFMoC.argtypes  = [POINTER(c_char), POINTER(c_char)]
	system.createLSFMoC.restype   = ctypes.c_int
	
	system.createELNMoC.argtypes  = [POINTER(c_char)]
	system.createELNMoC.restype   = ctypes.c_int
	
	system.connectMoC.argtypes    = [POINTER(c_char), POINTER(c_char), POINTER(c_char), POINTER(c_char), POINTER(c_char)]
	system.connectMoC.restype     = ctypes.c_int
		
	system.createTraceFile.argtypes  = [POINTER(c_char)]
	system.createTraceFile.restype   = ctypes.c_int
	
	system.traceSignal.argtypes  = [POINTER(c_char), POINTER(c_char)]
	system.traceSignal.restype   = ctypes.c_int
	
	
	return system
#Global Variables	
global headData
global model
global col,row,layer
global x
global y
global iFactor	
global edaMocFunc
global lookupTable

#Initialize model
modelName 	= "elder"
modelPath 	= os.path.join(os.getcwd(), 'models', 'SEAWAT', '3_elder')
modelOutputPath = commonFunctions.createModelOutputPath(modelName, "_temp")
model    	= seawat(modelName, modelPath, exePath, modelOutputPath + "\\")

#Initialize helper functions
edaMocFunc      = edaMoC()
commonFunctions = common()

#Load Elder Lookup table ( degree 8 )
lookupTable = commonFunctions.loadLUT(os.path.join(os.getcwd(), "LUT", "Elder","elder_ssm_degree_8.lut"))

#Function to change SEAWAT SSM data  (salt concentration)
def setSSMData(model, factor):
	for key, value in sorted(model.ssmData.iteritems()):
		tmp = str(key)
		tmp = tmp.replace('(', '')
		tmp = tmp.replace(')', '')
		tmp = tmp.replace("'", '')
		col, row, layer = tmp.split(',')
		col = int(col) -1
		row = int(row) -1
		layer = int(layer) -1
		model.initConcentration[col][row][layer] = float(model.initConcentration[col][row][layer]) * factor
		model.ssmData[key] = float(model.ssmData[key]) * factor

#Stub MoC		
def stub( time, I, Q):
	return 1

#Flow MoC	
def flow( time, I, Q):
	global iFactor
	#Set output data
	Q[0] = iFactor
	return 1

#HYDR or EDA MoC	
def MoC( time, I, Q):
	global model
	global headData
	global simulationTime 
	global timeStep
	global col,row,layer
	global edaMocFunc
	global useEDAModel
	
	#Input Q is ignore for now	
	#Calculate SEAWAT model at time 0	
	if time <= 0.0:	
		#Update simulation parameters
		model.PERLEN = simulationTime
		model.NSTP   = round(simulationTime/timeStep,1)
		if useEDAModel:
			#Use EDA model
			headData =  edaMocFunc.elder(model, lookupTable, I[0])
		else:
			#Use HYDR model
			setSSMData(model, I[0])
			model.runSim()
			headData = model.head
	else:
		#Send data
		Q[0] = headData[round(time, 1)][col][row][layer]
	return 1
	
#Monitor MoC	
def monitor( time, I, Q):
	#Collect data
	x.append(round(time, 1))
	y.append(I[0])
	return 1
	
def main():
	#Global variable reference
	global simulationTime 
	global timeStep
	global col,row,layer
	global x
	global y
	global useEDAModel
	global iFactor
	
	#Load systemC-AMS DLL
	system = loadSystemSimulator()
	
	#Initialize global variables
	iFactor 	= 0.0	
	x = []
	y = []
	
	#Col, row and layer to monitor
	col,row,layer = 22,0,0
	
	#Run simulation for 7300.0 cycles (Days, Hours, seconds ..etc)
	simulationTime 	   = 7300.0
	#Timesteps of 1825 cycles (Days, Hours, seconds ..etc)
	timeStep           = 1825
	
	#Create Signals
	system.createSignal( "z")
	system.createSignal( "Q1")
	system.createSignal( "Q2")
	system.createSignal( "Q3")
	
	#Create TDF Flow instance
	stubMoCProc = MoCFuncType(stub)
	system.createTDFMoC( "Stub",  c_double(timeStep), stubMoCProc)
	
	#Create TDF Flow instance
	flowMoCProc = MoCFuncType(flow)
	system.createTDFMoC( "Flow",  c_double(timeStep), flowMoCProc)
	
	#Create TDF MOC instance
	MoCProc       = MoCFuncType(MoC)
	system.createTDFMoC( "MoC",   c_double(timeStep), MoCProc)	
	
	#Create TDF Monitor instance
	monitorProc  = MoCFuncType(monitor)
	system.createTDFMoC( "Monitor",   c_double(timeStep), monitorProc)
	
	#Connect signals
	system.connectMoC( "Stub", 	"I",	"z",  "Q",  "z")
	system.connectMoC( "Flow", 	"I",	"z",  "Q",  "Q1")
	system.connectMoC( "MoC",	"I",	"Q1", "Q",  "Q2")
	system.connectMoC( "Monitor",   "I",	"Q2", "Q",  "Q3")
	
	#Trace Signals
	system.createTraceFile("data.txt")
	
	#Trace s1
	system.traceSignal("data.txt", "Q1")
	system.traceSignal("data.txt", "Q2")
	
	#Ask user whether to run the HYDR MoC or EDA MoC
	useEDAModel = commonFunctions.queryYesOrNoAnswerFromUser('Would you like to use EDA model?')
	
	inputs = numpy.arange(0.0, 1.05, 0.25)
	
	iFactor = float(raw_input("Enter Input factor: " + str(inputs) ))
	print iFactor
	
	#Run simulation
	elapsedTime =  time.clock()
	system.startSim(c_double(simulationTime + timeStep))
	elapsedTime =  time.clock() - elapsedTime
	
	#Print elapse time and final head data
	print "Elapse Time = " , elapsedTime, " Seconds"
	
	#Print Monitor data
	print "Col = ",col, "Row = ",row, "Layer = ",layer
	print "Time FinalHead"
	for index in range(0, len(x)):
		print x[index]," ", y[index]
	
if __name__ == '__main__':
    main()


