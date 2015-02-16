import sys
sys.path.append('..\modules')
import time
from common import common
from seawat import seawat
from hydrologicalSimulation import hydrologicalSimulation
from edaMoC import edaMoC
import numpy
import os

import shutil
import ctypes
from ctypes import *
from scipy import stats
from scipy.optimize import curve_fit

exePath   = os.path.join(os.getcwd(), 'models', 'SEAWAT', 'bin','swt_v4.exe')
commonFunctions = common()
edaMocFunc      = edaMoC()

	    
def main():
	
	print ".....1"
	#Ask the user wheather or not to regenerate the lookup tables for the SEAWAT benchmarks
	regenerateLUT = commonFunctions.queryYesOrNoAnswerFromUser('Would you like to regenerate the lookup tables?')
	
	print ".....2"
	#Run case 1 SEAWAT benchmark
	runCaseOneBenchmark(regenerateLUT)
	
	print ".....3"
	#Run henry SEAWAT benchmark
	runHenryBenchmark(regenerateLUT)
	
	print ".....4"
	#Run Elder SEAWAT benchmark
	runElderBenchmark(regenerateLUT)
	
def runCaseOneBenchmark(redo):

	modelName = "case1"
	modelPath = os.path.join(os.getcwd(), 'models', 'SEAWAT', '1_box','case1')
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")	
	
	layerInputs = []
	layerInputs.append(0.0)	
	layerInputs.append(0.625)
	layerInputs.append(1.0)	
	layerInputs.append(2.0)	
	layerInputs.append(3.0)	
	
	#Add column description
	commonFunctions.addColDes("Layer 16")
	reGen = redo
	for i in range(0, len(layerInputs)):
		layerData = numpy.zeros( model.NLAY, dtype=c_float, order='C')
		
		for ly in range(0, model.NLAY):
			layerData[ly] = model.initHead[0][0][ly]
		
		layerData[16] = layerInputs[i]
		
		genModelHCModel(modelName, modelPath, modelOutputPath, model, hydroSim, reGen, layerData)
		reGen = False
	
	#Remove tempory directory
	shutil.rmtree(os.path.join(os.getcwd(), modelName + "_tmp"))
	
def runHenryBenchmark(redo):
	modelName = "henry"
	modelPath = os.path.join(os.getcwd(), 'models', 'SEAWAT', '2_henry','1_classic_case1')
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")
	
	wellInputs = []		
	wellInputs.append(0.4)
	wellInputs.append(0.5)
	wellInputs.append(0.6)
	wellInputs.append(0.7)
	
	#Add column description
	commonFunctions.addColDes("Well Input")
	
	reGen = redo
	for i in range(0, len(wellInputs)):
		genModelWellModel(modelName, modelPath, modelOutputPath, model, hydroSim, reGen, wellInputs[i])
		reGen = False
	
	#Remove tempory directory
	shutil.rmtree(os.path.join(os.getcwd(), modelName + "_tmp"))
	
def runElderBenchmark(redo):

	modelName = "elder"
	modelPath = os.path.join(os.getcwd(), 'models', 'SEAWAT', '3_elder')
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")	
	degree = 2
	newModel = []
	
	#List of inputs to create polynomial coefficients
	inputs = numpy.arange(0.0, 1.05, 0.05)
	lutName = modelName + "_ssm_degree_"+str(degree)+".lut"	
	outputLutPath  = os.path.join(os.getcwd(), 'LUT')
	lutPathFile = os.path.join(outputLutPath, modelName)
	lutFileName = os.path.join(lutPathFile, lutName)
	lookupTable = commonFunctions.loadLUT(lutFileName)
	
	if len(lookupTable) == 0 or redo == True:
		for ix in range (0, len(inputs)):
			factor = inputs[ix]
			model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp" )
			setSSMData(model, factor)
			originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
			newModel.append(originalModelHead)
			
		lutPathFile = commonFunctions.createOutputPath(outputLutPath, modelName)	
		
		for deg in (2, 4, 8, 16):
			print "Generate ", modelName, " degree ", deg," Lookup table"
			lutName      = modelName + "_ssm_degree_"+str(deg)+".lut"	
			lookupTable  = createSSMModel(model, newModel, inputs, deg)
			lutFileName  = os.path.join(lutPathFile, lutName)
			commonFunctions.storeLUT(lutFileName, lookupTable)
			
	lutName = modelName + "_ssm_degree_"+str(degree)+".lut"	
	
	factor  =  1.0
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")
	setSSMData(model, factor)
	originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
	
	commonFunctions.saveModel(model, modelName, "Original", "EDA", originalModelHead, hydroSim)
	
	
	rt = {}
	rr = {}
	cnt = 0
	
	#Add column description
	commonFunctions.addColDes("Degree")
		
	for degree in (2,4,8,16):
		
		lutName = modelName + "_ssm_degree_"+str(degree)+".lut"	
		lutPathFile = os.path.join(os.path.join(os.getcwd(), 'LUT'),  modelName)
		lutFileName = os.path.join(lutPathFile, lutName)
		lookupTable = commonFunctions.loadLUT(lutFileName)
		newModelTime, newModelHead = commonFunctions.runEDAModelWithLUT(modelName, model, edaMocFunc.elder, lookupTable, factor)
		commonFunctions.saveModel(model, modelName, "degree" + str(degree), "EDA", newModelHead, hydroSim)
		
		#Analyze Data
		commonFunctions.analyzeData(degree, model, modelName, originalModelTime, originalModelHead, newModelTime, newModelHead)
		rt = {}
		rr = {}
				
		rt[degree] = newModelHead
		rr[degree] = commonFunctions.minRsquared
		
		title  = str(cnt) + '. Elder Model Head Versus Time Plot at Cell x = '
		title += str(commonFunctions.imin) + ' y =' 
		title += str(commonFunctions.jmin) + ' z = '
		title += str(commonFunctions.kmin)
		cnt   += 1
		
		commonFunctions.plotCellsOverTime(modelName, title, originalModelHead, rt, commonFunctions.imin, commonFunctions.jmin, commonFunctions.kmin, rr)
	
	#Remove tempory directory
	shutil.rmtree(os.path.join(os.getcwd(), modelName + "_tmp"))
	
def runInterpolationBenchmark(redo):

	modelName = "elder"
	modelPath = os.path.join(os.getcwd(), 'models', 'SEAWAT', '3_elder')
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")	
	degree = 8
	newModel = []
	lutPrefix = "_inter_nPoints_"
	#List of inputs to create polynomial coefficients
	inputs = numpy.arange(0.0, 1.05, 0.05)
	lutName = modelName + lutPrefix+str(degree)+".lut"	
	outputLutPath  = os.path.join(os.getcwd(), 'LUT')
	lutPathFile = os.path.join(outputLutPath, modelName)
	lutFileName = os.path.join(lutPathFile, lutName)
	lookupTable = commonFunctions.loadLUT(lutFileName)
	
	if len(lookupTable) == 0 or redo == True:
		for ix in range (0, len(inputs)):
			factor = inputs[ix]
			model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp" )
			setSSMData(model, factor)
			originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
			newModel.append(originalModelHead)
			
		lutPathFile = commonFunctions.createOutputPath(outputLutPath, modelName)	
		
		for deg in (8,16):
			print "Generate ", modelName, " points ", deg," Lookup table"
			lutName      = modelName + lutPrefix +str(deg)+".lut"	
			lookupTable  = createInterpolationModel(model, newModel, inputs, deg)
			lutFileName  = os.path.join(lutPathFile, lutName)
			commonFunctions.storeLUT(lutFileName, lookupTable)
			
	lutName = modelName + lutPrefix +str(degree)+".lut"	
	
	factor  =  1.0
	model, hydroSim, modelOutputPath = initModel(modelName, modelPath, "_tmp")
	setSSMData(model, factor)
	originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
	
	commonFunctions.saveModel(model, modelName, "Original", "EDA", originalModelHead, hydroSim)
	
	
	rt = {}
	rr = {}
	cnt = 0
	
	#Add column description
	commonFunctions.addColDes("Points")
		
	for degree in (8,16):
		
		lutName = modelName + lutPrefix+str(degree)+".lut"	
		lutPathFile = os.path.join(os.path.join(os.getcwd(), 'LUT'),  modelName)
		lutFileName = os.path.join(lutPathFile, lutName)
		lookupTable = commonFunctions.loadLUT(lutFileName)
		newModelTime, newModelHead = commonFunctions.runEDAModelWithLUT(modelName, model, edaMocFunc.interpolationMoC, lookupTable, factor)
		commonFunctions.saveModel(model, modelName, "point" + str(degree), "EDA", newModelHead, hydroSim)
		
		#Analyze Data
		commonFunctions.analyzeData(degree, model, modelName, originalModelTime, originalModelHead, newModelTime, newModelHead)
		rt = {}
		rr = {}
				
		rt[degree] = newModelHead
		rr[degree] = commonFunctions.minRsquared
		
		title  = str(cnt) + '. Elder Model Head Versus Time Plot at Cell x = '
		title += str(commonFunctions.imin) + ' y =' 
		title += str(commonFunctions.jmin) + ' z = '
		title += str(commonFunctions.kmin)
		cnt   += 1
		
		commonFunctions.plotCellsOverTime(modelName, title, originalModelHead, rt, commonFunctions.imin, commonFunctions.jmin, commonFunctions.kmin, rr)
	
	#Remove tempory directory
	shutil.rmtree(os.path.join(os.getcwd(), modelName + "_tmp"))
	
def initModel(modelName, modelPath, prefix):
	modelOutputPath = commonFunctions.createModelOutputPath(modelName, prefix)
	model    	= seawat(modelName, modelPath, exePath, modelOutputPath + "\\")
	hydroSim 	=  hydrologicalSimulation(model.NCOL, model.NROW, model.NLAY)
	return model, hydroSim, modelOutputPath
	
def setWellModelInput(model, input):
	if os.path.exists(model.welFilePath):	
		for key, value in sorted(model.welData.iteritems()):
			model.welData[key] = input
			
def genModelHCModel(modelName, modelPath, modelOutputPath, model, hydroSim, redo, layerData):

	for l in range(0, model.NLAY):
		hydroSim.setLayer(model.initHead, model.NCOL, model.NROW, l-1, layerData[l] )
		
	originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
	commonFunctions.saveModel(model, modelName, "HYDR_layerData_" + str(layerData[16]), "EDA", originalModelHead, hydroSim)

	lutName     = modelName + "_hc.lut"
	outputLutPath = os.path.join(os.getcwd(), 'LUT')
	lutPathFile   = os.path.join(outputLutPath, modelName)
	lutFileName   = os.path.join(lutPathFile, lutName)
	hcLUT         = commonFunctions.loadLUT(lutFileName)
	
	if len(hcLUT) == 0 or redo == True:
		print "Generate ", modelName, "Lookup table"
		lutPathFile = commonFunctions.createOutputPath(outputLutPath, modelName)
		hcLUT = prepareHeadAndConcentrationLutTable(modelName, modelPath, exePath, modelOutputPath, hydroSim)		
		commonFunctions.storeLUT(lutFileName, hcLUT)
		
	newModelTime, newModelHead = commonFunctions.runEDAModelWithLUT(modelName, model, edaMocFunc.caseOne, hcLUT, 0)
	
	#Save final head
	commonFunctions.saveModel(model, modelName, "EDA_layerData_" + str(layerData[16]), "EDA", newModelHead, hydroSim)

	#Analyze Data
	commonFunctions.analyzeData(layerData[16], model, modelName, originalModelTime, originalModelHead, newModelTime, newModelHead)
 
def genModelWellModel(modelName, modelPath, modelOutputPath, model, hydroSim, redo, wellInput):

	setWellModelInput(model, wellInput)
	originalModelHead, originalModelTime = commonFunctions.runOriginalModel(model)
	commonFunctions.saveModel(model, modelName, "HYDR_wellInput_" + str(wellInput), "EDA", originalModelHead, hydroSim)
	
	if os.path.exists(model.welFilePath):
		
		lutName     = modelName + "_well.lut"
		outputLutPath = os.path.join(os.getcwd(), 'LUT')
		lutPathFile = os.path.join(outputLutPath, modelName)
		lutFileName = os.path.join(lutPathFile, lutName)
		wellLUT = commonFunctions.loadLUT(lutFileName)
		
		if len(wellLUT) == 0 or redo == True:
			print "Generate ", modelName, "Lookup table"
			lutPathFile = commonFunctions.createOutputPath(outputLutPath, modelName)
			wellLUT = prepareWellLutTable(modelName, modelPath, exePath, modelOutputPath, hydroSim)
			commonFunctions.storeLUT(lutFileName, wellLUT)
	
	newModelTime, newModelHead = commonFunctions.runEDAModelWithLUT(modelName, model, edaMocFunc.henry, wellLUT, wellInput)
	
	#Save final head
	commonFunctions.saveModel(model, modelName, "EDA_wellInput_" + str(wellInput), "EDA", newModelHead, hydroSim)
	
	#Analyze Data
	commonFunctions.analyzeData(wellInput, model, modelName, originalModelTime, originalModelHead, newModelTime, newModelHead)
		
def diffHeads(h, c, a, NLAY, NROW, NCOL):
	diff = {}
	for time, value in sorted(h.iteritems()) :
		diff[time] = c[time] + h[time]
		diff[time] = a[time] - diff[time] 
		
	return diff		
			
def prepareHeadAndConcentrationLutTable(modelName, modelPath, exePath, modelOutputPath, hydroSim):
	
	model = seawat(modelName, modelPath, exePath, modelOutputPath)

	hydroSim.initArray(model.initHead, 	        model.NCOL,	model.NROW,	model.NLAY,	0.0)
	hydroSim.initArray(model.initConcentration,	model.NCOL,	model.NROW,	model.NLAY,	0.0)
	hydroSim.clearWellData(model)
			
	HfLUT = []
	
	for layer in range(0, model.NLAY):
		if layer > 0:
			hydroSim.setLayer(model.initHead, model.NCOL, model.NROW, layer-1, 0.0)
		hydroSim.setLayer(model.initHead, model.NCOL, model.NROW, layer,1.0)
		model.runSim()
		HfLUT.append(model.head)
	
	model = seawat(modelName, modelPath, exePath, modelOutputPath)

	hydroSim.initArray(model.initHead, 	        model.NCOL,	model.NROW,	model.NLAY,	0.0)
	hydroSim.clearWellData(model)
	model.runSim()
	CLUT = model.head
	
	diffLUT = []
	model = seawat(modelName, modelPath, exePath, modelOutputPath)

	hydroSim.initArray(model.initHead, 	        model.NCOL,	model.NROW,	model.NLAY,	0.0)
	
	for layer in range(0, model.NLAY):
		hydroSim.initArray(model.initHead, 	        model.NCOL,	model.NROW,	model.NLAY,	0.0)
		hydroSim.clearWellData(model)
		hydroSim.setLayer(model.initHead, model.NCOL, model.NROW, layer, 1.0)
		model.runSim()	
		diffLUT.append(diffHeads(HfLUT[layer], CLUT, model.head, model.NLAY, model.NROW, model.NCOL))
	
	lookupTable = {}
	
	lookupTable['CLUT']    = CLUT
	lookupTable['HfLUT']   = HfLUT
	lookupTable['diffLUT'] = diffLUT
	
	hf   = []
	diff = []

	for time, value in sorted(CLUT.iteritems()):
		for ly in range (0, model.NLAY):	
			hf.append(HfLUT[ly][time])
			diff.append(diffLUT[ly][time])
		
	lookupTable['hf']   = numpy.array(hf)
	lookupTable['diff'] = numpy.array(diff)
	
	
	return lookupTable

def prepareWellLutTable(modelName, modelPath, exePath, modelOutputPath, hydroSim):

	model = seawat(modelName, modelPath, exePath, modelOutputPath)

#for debug purposes	
#	for key, value in sorted(model.welData.iteritems()):
#			print "init=",key , " = ",model.welData[key]
			
	inputs = numpy.linspace(0.4, 0.7, num = 50)
	f     = {}
	fdiff = {}
	
	for ix in range (0, len(inputs)):
	        x = inputs[ix]
		model = seawat(modelName, modelPath, exePath, modelOutputPath)
		for key, value in sorted(model.welData.iteritems()):
			model.welData[key] = x	
		model.runSim()
		f[x] = model.head
	
	for ix in range (0, len(inputs)):
	        x = inputs[ix]
		model = seawat(modelName, modelPath, exePath, modelOutputPath)
	
		for key, value in sorted(model.welData.iteritems()):
			model.welData[key] = x	
		model.runSim()
		fdiff[x] = model.head
		
		for time, value in sorted(model.head.iteritems()):
			fdiff[x][time] = model.head[time]- f[x][time]

		
	outputs  = {}
	outputs1 = {}
	
	
	for time, value in sorted(model.head.iteritems()):
		outputs[time] = {}
		outputs1[time] = {}
		for kk in range (0,model.NLAY):
			for jj in range (0, model.NROW):
				for ii in range (0, model.NCOL):
					outputs[time][ii, jj, kk] = []
					outputs1[time][ii, jj, kk] = []
	
	for input, data in sorted(f.iteritems()):
		for time, out in sorted(data.iteritems()):
			for kk in range (0, model.NLAY):
				for jj in range (0, model.NROW):
					for ii in range (0, model.NCOL):
						outputs[time][ii, jj, kk].append(out[ii, jj, kk])
	
	for input, data in sorted(fdiff.iteritems()):
			for time, out in sorted(data.iteritems()):
				for kk in range (0, model.NLAY):
					for jj in range (0, model.NROW):
						for ii in range (0, model.NCOL):
							outputs1[time][ii, jj, kk].append(out[ii, jj, kk])
						
	lookupTable = {}
	NLAY = model.NLAY
	NROW = model.NROW
	NCOL = model.NCOL
	
	#Linear regression (degree 1)
	degree = 1
	P    = numpy.zeros(NLAY * NROW * NCOL*(degree + 1 )).reshape(NLAY, NROW, NCOL, degree + 1)
	
	#Calculate linear regression coeffients
	for time, value in sorted(outputs.iteritems()):
		for kk in range (0,  model.NLAY):
			for jj in range (0,  model.NROW):
				for ii in range (0,  model.NCOL):									
					fit = numpy.polyfit( numpy.array(inputs), numpy.array(outputs[time][ii, jj, kk]), degree )
					P[kk,jj,ii] = fit
					
	MoC = {}
	MoC['P']            = P
	MoC['degree']       = degree
	lookupTable['MoC1'] = MoC
			
	return lookupTable
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
	
def poly3DRegression(data , input, time_len, degree, NCOL, NROW, NLAY):
	regCoeffs = numpy.zeros(NLAY * NROW * NCOL*(degree+1)).reshape(NLAY, NROW,NCOL, degree+1)
	for col in range(data.shape[0] ) :
		for row in range(data.shape[1] ) :
			for layer in range(data.shape[2] ):
				fit = numpy.polyfit( input, data[col,row,layer,:], degree )
				regCoeffs[layer,row,col] = fit
	return regCoeffs	
				
def createSSMModel(model, newModel, inputs, degree):

	maxNumberOfModels = len(newModel)
	coeffTbl  = {}
	x = []
	
	for time, value in sorted(newModel[0].iteritems()) :
		x.append(time)
	
	numberOfTimeSteps = len(x)
	NCOL = model.NCOL 
	NROW = model.NROW
	NLAY = model.NLAY
	
	x1 = []
	
	for n in range(0, len(newModel)):
		x1.append(inputs[n])
		
	for n in range(0, len(newModel)):
		inp = inputs[n]
		head  = numpy.zeros( NCOL*NROW*NLAY * numberOfTimeSteps, dtype=c_float, order='C').reshape(NCOL, NROW, NLAY, numberOfTimeSteps)			
		for kk in range (0, NLAY):
			for jj in range (0, NROW):
				for ii in range (0, NCOL):
					t = 0
					y = []
					for time, value in sorted(newModel[n].iteritems()):
						head[ii][jj][kk][t] = value[ii, jj, kk]						
						y.append(value[ii, jj, kk])	
						t+=1
						
		coeffTbl[str(inp)] = poly3DRegression(head , x, numberOfTimeSteps, degree, NCOL, NROW, NLAY)	

	lookupTable = {}
	MoC = {}
	MoC['coeffTbl']     = coeffTbl
	lookupTable['MoC1'] = MoC
	
	return lookupTable
	
if __name__ == '__main__':
    main()


