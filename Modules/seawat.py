import re
import binascii
import struct
import numpy
from subprocess import Popen, PIPE
import time
import os
import os.path
from string import lowercase
from itertools import groupby
from parseData import parseData
from ctypes import *
"""
SEAWAT Wrapper module
"""
class seawat:

	#Initializes SEAWAT wrapper module
	def __init__(self, modelName, modelPath, exePath, modelOutputPath):		
		
		#Populate module information
		self.exePath = exePath
		self.modelPath = modelPath
		self.modelName = modelName
		self.namFile = "seawat.nam";
		
		#Parse SEAWAT configuration file
		self.parseData = parseData();
		
		#Retrive NCOL, NROW, and NLAY information
		self.getDimensions()
		
		#Set the SEAWAT output path
		self.modelOutputPath = modelOutputPath
		
		#Initialize fresh and salt water heads
		self.initHead = {}
		self.initConcentration = {}
		self.initBottom = {}
	
		#Retrive SEAWAT file path
		basInputFilePath  = os.path.join(self.modelPath, self.modelName + ".bas")
		btnInputFilePath  = os.path.join(self.modelPath, self.modelName + ".btn")
		bcfInputFilePath  = os.path.join(self.modelPath, self.modelName + ".bcf")
		self.welFilePath = os.path.join(self.modelPath, self.modelName + ".wel")
		self.ssmFilePath = os.path.join(self.modelPath, self.modelName + ".ssm")
		
		#Retrive data if the file exist
		if os.path.exists(self.welFilePath):
			self.welData = self.getWellData(self.welFilePath)
		
		#Retrive data if the file exist
		if os.path.exists(self.ssmFilePath):
			self.ssmData = self.getssmData(self.ssmFilePath)
		
		#Retrieve fresh water heads
		self.initHead = self.getLayerData(basInputFilePath, "HEAD")
		
		#Retrieve salt water heads
		self.initConcentration = self.getLayerData(btnInputFilePath, "CONCENTRATION")
		
		#Retrieve porosity information
		self.porosity = self.getLayerData(btnInputFilePath, "POROSITY")
		
		#Retrieve transmitivity information
		self.transmitivity = self.getLayerData(bcfInputFilePath, "TRANSMISSIVITY")

		#Get SEAWAT file list
		seawatFileList = self.getSeawatFileNames()
		self.copyFile(self.namFile)
		
		#Default SEAWAT simulation time value
		self.simTime = 0
		
		#Copy SEAWAT file to new location
		for key, value in sorted(seawatFileList.iteritems()) :
			self.copyFile(value)

	#Get initial parameters
	def getInitParams(self):
		return self.initHead, self.initConcentration, self.initBottom
	
	#Insert spaces
	def insert_space(self, nSpaces):
		space = ""	
		for sp in range(nSpaces):
			space = space + " "
		return space	
	
	#Insert data		
	def insertData(self,param, spaceCount):

		line = ""
		sp = 0
		nSpaceCountSize = len(spaceCount)

		for i in range(len(param)):
			spaces = 0
			if sp < nSpaceCountSize:
				spaces = spaceCount[sp]
			
			line = line + param[i] + self.insert_space(spaces)
			sp=sp+1

		return line
	
		
	#Runs SEAWAT simulation
	def runSim(self):
		
		#Retrive input paths	
		basInputFilePath  = os.path.join(self.modelPath, self.modelName + ".bas")
		basOutputFilePath = os.path.join(self.modelOutputPath, self.modelName + ".bas")
		
		btnInputFilePath  = os.path.join(self.modelPath, self.modelName + ".btn")
		btnOutputFilePath = os.path.join(self.modelOutputPath, self.modelName + ".btn")
		
		bcfInputFilePath  = os.path.join(self.modelPath, self.modelName + ".bcf")
		bcfOutputFilePath = os.path.join(self.modelOutputPath, self.modelName + ".bcf")
		
		#Modify data	
		self.modifyData("HEAD", basInputFilePath, basOutputFilePath, self.initHead)
		self.modifyData("CONCENTRATION", btnInputFilePath, btnOutputFilePath, self.initConcentration)
		
		self.modifyData("POROSITY", btnOutputFilePath, btnOutputFilePath, self.porosity)
		self.modifyData("TRANSMISSIVITY", bcfInputFilePath, bcfOutputFilePath, self.transmitivity)
		
		#Retrive well input path
		welInputFilePath  = os.path.join(self.modelPath, self.modelName + ".wel")
		welOutputFilePath = os.path.join(self.modelOutputPath, self.modelName + ".wel")
		
		#Check if file exist
		if os.path.exists(welInputFilePath):
			self.modifyWellData(welInputFilePath,welOutputFilePath, self.welData)
		
		ssmInputFilePath  = os.path.join(self.modelPath, self.modelName + ".ssm")
		ssmOutputFilePath = os.path.join(self.modelOutputPath, self.modelName + ".ssm")
		
		#Check if file exist
		if os.path.exists(ssmInputFilePath):
			self.modifySSMData(ssmInputFilePath, ssmOutputFilePath, self.ssmData)
			
		self.ssmFilePath = os.path.join(self.modelPath, self.modelName + ".ssm")
		
		#Update simulation time
		self.changeSimTime()
		
		#Run SEAWAT simulation
		self.runSeawat()
		
		#Retrieve final heads
		self.head = self.readHDS(os.path.join(self.modelOutputPath, self.modelName + ".hds"))
	
	#Set value in array
	def setVal( self,data,layer,value):
		for jj in range(self.NROW):
			for ii in range(self.NCOL):			         
				data[ii][jj][layer] = value
	
	#Set layer data		
	def setLayerData(self,data,layer,param,nParams):
		
		ii = self.layerColInit
		jj = self.layerRowInit
		
		for i in range(nParams):
			if self.parseData.isFloatNum(param[i]):
				value = self.parseData.getFloatNum (param[i])	
				
				data[ii][jj][layer] = value
				ii = ii + 1
				if ii == self.NCOL:
					ii = 0
					jj =  jj + 1
					if jj == self.NROW:
						jj = 0
		
		self.layerColInit = ii
		self.layerRowInit = jj
		
	#Get layer parameters			
	def getLayerParam(self,data,layer):

		str = "  "
		cnt = 0;
		skipLines = 0
		
		for jj in range(self.NROW):
			for ii in range(self.NCOL):
				str += self.parseData.eformat(data[ii][jj][layer], 2, 4)
				cnt+=1
				
				if  cnt == 10 :
					str=str + "\n  "
					skipLines+=1
					cnt = 0	
				elif ii < (self.NCOL -1) :
					str=str + "  "
		return str + "\n", skipLines
	
	#Retrives parameter from file Data array
	def getFileCurrentParams(self,fileData, l):
			line    	= fileData[l][0]
			mask    	= fileData[l][1]
			param   	= fileData[l][2]
			nParams 	= fileData[l][3]
			
			return line, mask, param, nParams
	
	def getFileNextParams(self, fileData, l):
			nextMask 	= fileData[l+1][1]
			nextParam   	= fileData[l+1][2]
			nextNparams 	= fileData[l+1][3]
			
			return nextMask, nextParam, nextNparams
			
	#Check data parameters				
	def checkDataParam(self,param,nParams):
		for i in range(nParams):
				param[i] = "%6.3f" % data[ii][jj][layer-1]
				ii = ii + 1
				if ii == self.NCOL:
					ii = 0
					jj =  jj + 1
	#Get Well data
	def getWellData(self, filename):
		
		fileData = self.readFileData(filename);
		n = len(fileData);
		data = {}
		
		for l in range(n-1):
			line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
			nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)

			if mask == "sii":
				self.NPWEL = param[1]
				self.MXL= param[2]
				#print "NPWEL = ", self.NPWEL, "MXL = ", self.MXL
			elif mask == "ii":
				self.MXACTW =  param[0]
				self.IWELCB =  param[1]
				#print "MXACTW = ", self.MXACTW, "IWELCB = ", self.IWELCB
			elif mask == "iissi":
				self.ITMP =  param[0]
				self.NP   =  param[1]
				#print "ITMP = ", self.ITMP,"NP = ", self.NP
			elif mask == "iiifi":
				layer  = param[0]
				row    = param[1]
				column = param[2]
				q      = param[3]
				data[column,row,layer] = q
				
		return data
	#Get SSM data	
	def getssmData(self, filename):
		
		fileData = self.readFileData(filename);
		n = len(fileData);
		data = {}
		
		for l in range(n-1):
			line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
			nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)
			if mask == "ssssss":
				self.FWEL = param[0]
				self.FDRN = param[1]
				self.FRCH = param[2]
				self.FEVT = param[3]
				self.FRIV = param[4]
				self.FGHB = param[5]
			
			if mask == "iiifif":
				#print "Mask = ",mask ," Line = ", line
				layer  = param[0]
				row    = param[1]
				column = param[2]
				q      = param[3]
				data[column,row,layer] = q
				
		return data
	
	#Modify SSM data	
	def modifySSMData(self, inputFile, outputFile, data):
		
		fileData = self.readFileData(inputFile);
		n = len(fileData);
		output = open(outputFile, "w")
		for l in range(n-1):
			line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
			nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)
			if mask == "iiifif":
				layer  = param[0]
				row    = param[1]
				column = param[2]
				q      = param[3]
				newQ = self.parseData.eformat(float(data[column,row,layer]), 2, 3)
				line = line.replace(q, newQ)
				q      = param[5]
				newQ = self.parseData.eformat(float(data[column,row,layer]), 3, 5)
				line = line.replace(q, newQ)
			print >> output, line,
			
		output.close()	
		
		return 		

	#Modify WELL data		
	def modifyWellData(self, inputFile, outputFile, data):
		
		fileData = self.readFileData(inputFile);
		n = len(fileData);
		output = open(outputFile, "w")
		
		for l in range(n-1):
			line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
			nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)
			
			if mask == "sii":
				self.NPWEL = param[1]
				self.MXL   = param[2]
			elif mask == "ii":
				self.MXACTW =  param[0]
				self.IWELCB =  param[1]
			elif mask == "iissi":
				self.ITMP =  param[0]
				self.NP   =  param[1]
			elif mask == "iiifi":
				layer  = param[0]
				row    = param[1]
				column = param[2]
				q      = param[3]
				newQ = "%2.7f" % float(data[column,row,layer])
				line = line.replace(q, newQ)
				
			print >> output, line,
			
		output.close()	
		
		return 		

	#Get layer data			
	def getLayerData(self, filename, name):
	
	    data = numpy.zeros((self.NCOL,self.NROW,self.NLAY), dtype=c_float, order='C')
	    fileData = self.readFileData(filename)
	    n = len(fileData)

	    for l in range(n-1):

		line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
		nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)
			
		layer = 0		
		if re.match("(.*)"+name+"(.*)", line):
			layer = int(param[nParams-1])
			
			
			if mask == "ifissi" and nextMask == "ifissi":
				value = self.parseData.getFloatNum (param[1])
				self.setVal(data,layer-1,float(value))
			elif mask == "ifissi" and nextMask == ( 'f' * ( 10 ) ):
				totalData = 0
				totalParam = nextParam
				
				self.layerColInit = 0
				self.layerRowInit = 0
				
				while ( totalData < (self.NCOL*self.NROW)):
					l = l + 1
					nextMask 	= fileData[l+1][1]
					nextParam       = fileData[l][2]
					nextNparams     = fileData[l][3]
					self.setLayerData(data, layer-1, nextParam, nextNparams)
					totalParam      = totalParam + nextParam
					totalData       = totalData + nextNparams
					
				self.setLayerData(data, layer-1, totalParam, len(totalParam))
					
			elif mask == "ifissi" and layer == self.NLAY:
				value = self.parseData.getFloatNum (param[1])
				self.setVal(data,layer-1,float(value))
			elif mask == "ifisssi" and nextMask == "ifisssi":
				value = self.parseData.getFloatNum (param[1])
				self.setVal(data,layer-1,float(value))
			elif mask == "ifisssi" and nextMask == ( 'f' * ( 10 ) ):
				totalData = 0
				totalParam = nextParam
				
				self.layerColInit = 0
				self.layerRowInit = 0
				
				while ( totalData < (self.NCOL*self.NROW)):
					l = l + 1
					nextMask    = fileData[l+1][1]
					nextParam   = fileData[l][2]
					nextNparams = fileData[l][3]
					self.setLayerData(data, layer-1, nextParam, nextNparams)
					totalParam = totalParam + nextParam
					totalData  = totalData + nextNparams
				self.setLayerData(data, layer-1, totalParam, len(totalParam))
					
			elif mask == "ifisssi" and layer == self.NLAY:
				value = self.parseData.getFloatNum (param[1])
				self.setVal(data,layer-1,float(value))
				
			elif mask == "fisssi" and ( nextMask == "fisssi" or  nextMask == "ifisssi"):
				value = self.parseData.getFloatNum(param[0])
				self.setVal(data,layer-1,float(value))
			elif mask == "fisssi" and nextMask == ( 'f' * ( self.NCOL * self.NROW ) ):
				totalData = 0
				totalParam = nextParam
				self.layerColInit = 0
				self.layerRowInit = 0
				while ( totalData < (self.NCOL*self.NROW)):
					l = l + 1
					nextMask    = fileData[l+1][1]
					nextParam   = fileData[l][2]
					nextNparams = fileData[l][3]
					self.setLayerData(data, layer-1, nextParam, nextNparams)
					totalParam = totalParam + nextParam
					totalData  = totalData + nextNparams

				self.setLayerData(data, layer-1, totalParam, len(totalParam))
				
			elif mask == "fisssi" and layer == self.NLAY:
				value = self.parseData.getFloatNum (param[0])
				self.setVal(data,layer-1,float(value))
			elif mask == "fissi" and ( nextMask == "fissi" or  nextMask == "ifissi"):
				value = self.parseData.getFloatNum (param[0])
				self.setVal(data,layer-1,float(value))
			elif mask == "fissi" and  layer == self.NLAY:
				value = self.parseData.getFloatNum (param[0])
				self.setVal(data,layer-1,float(value)) 
	    return data
	    
	#Get dimentions
	def getDimensions(self):
	    disInputFilePath  = os.path.join(self.modelPath, self.modelName + ".dis")
	    file = open(disInputFilePath, "r")
	    name = "BOTTOM"
	    self.initBottom = []
	    
	    for line in file:
		mask, nParams, param = self.parseData.getMask (line)
		check = True

		if mask == "iiiiii" and nParams == 6:
			self.NLAY	=  int(param[0])
			self.NROW 	=  int(param[1])
			self.NCOL 	=  int(param[2])
			self.NPER 	=  int(param[3])
			self.ITMUNI 	=  int(param[4])
			self.LENUNI 	=  int(param[5])
			
		elif mask == "fifs" and nParams == 4:
			self.PERLEN	=  float(param[0])
			self.NSTP 	=  int(param[1])
			self.TSMULT 	=  float(param[2])
			self.SS 	=  param[3]

		if re.match("(.*)"+name+"(.*)", line):
		    layer = int(param[nParams -1])
		    for i in range(nParams): 
			if self.parseData.isFloatNum(param[i]):
			    self.initBottom.append( float(self.parseData.getFloatNum (param[i])) )
	    
	    file.close()
	    
	def changeSimTime(self):
	
		outputFile = self.modelOutputPath + self.modelName + ".dis"
		output = open(outputFile, "w")
		
		disInputFilePath  = os.path.join(self.modelPath, self.modelName + ".dis")
		
		file = open(disInputFilePath, "r")
		
		for line in file:
			mask, nParams, param = self.parseData.getMask (line)
			check = True
			if mask == "fifs" and nParams == 4:
				tmp = "%5.6f" % self.PERLEN
				line = line.replace(param[0], tmp)
				tmp = "%d" % self.NSTP
				line = line.replace(param[1], tmp)

			print >> output, line,	
		file.close()

		output.close()
		
	def displayLayer(filename, name):
	    file = open(filename, "r")
	    for line in file:    
		if re.match("(.*)"+name+"(.*)", line):
		    print line
	    file.close()
    
	def copyFile(self, inputFile):
	
		inputFilePath  = os.path.join(self.modelPath, inputFile)
		file = open(inputFilePath, "r")
		
		outputFile = os.path.join(self.modelOutputPath, inputFile)
		
		output = open(outputFile, "w")
		
		for line in file:
			print >> output, line,	    
		file.close()
		output.close()

	def getSeawatFileNames(self):
	
		namInputFilePath  = os.path.join(self.modelPath, self.namFile)
		file = open(namInputFilePath, "r")
		seawatFileList = {}
		
		for line in file:
			if line.find("#") == -1:
				l = line.split();
				if l[0] != "DATA(BINARY)" and l[0] != "GLOBAL" and l[0] != "LIST":
					seawatFileList[l[0]] = l[2]
		
		file.close()
		return seawatFileList
	def readFileData(self, inputFile):
	
		fileData =[]
		file = open(inputFile, "r")
		for line in file:
		
			mask, nParams, param = self.parseData.getMask(line)
			fileData.append([line,mask,param,nParams])
		mask, nParams, param = self.parseData.getMask("# End of File")
		fileData.append([line,mask,param,nParams])

		return fileData
		
	def splitBySec(self, str, num):
		return [ str[start:start+num] for start in range(0, len(str), num) ]
	
	def modifyData(self, name, inputFile, outputFile, data):

		fileData = self.readFileData(inputFile)
		n = len(fileData)
		output = open(outputFile, "w")
		for l in range(n-1):

			line, mask, param, nParams       =  self.getFileCurrentParams(fileData, l)
			nextMask, nextParam, nextNparams =  self. getFileNextParams(fileData, l)
			layer = 0
			
			if mask == "ff":
				value = float(self.parseData.getFloatNum (param[0]))
				
				if value > 0:

					tmp = self.parseData.eformat(self.PERLEN, 2, 4)
					paramBTN = self.splitBySec(line,10)
					tmp1 = "%10d" % self.NSTP
					
					line = line.replace(paramBTN[1], tmp1)
					line = line.replace(param[0], tmp)
					
			if re.match("(.*)"+name+"(.*)", line):
				layer = int(param[nParams-1])
				if name == "TRANSMISSIVITY":
					if mask == "ifissi" and nextMask == "ffffffffff":
						tmp1 = line
						tmp2, skipLines = self.getLayerParam(data,layer-1)
						tmp1 += line

				if mask == "ifisssi" and nextMask == "ifisssi":
					value = self.parseData.getFloatNum (param[1])
					format = self.parseData.getFloatFormat (param[1])
					if nParams == 6:
						tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					else:
						tmp = "%6.3f" % data[0][0][layer-1]
					param[1] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']				
					line = line.replace(value, tmp)
					
					spaceCount = counts[1:len(counts)]
					line = self.insert_space(9) + self.insertData(param, spaceCount)+"\n"

				elif mask == "ifisssi" and nextMask == "ffffffffff":
					
					tmp1 = line
					line, skipLines = self.getLayerParam(data,layer-1)
					tmp1 += line
					for ln in range(skipLines + 1):					
						fileData[l+1+ln][0] = ""
					
					line = tmp1
					
				elif mask == "ifisssi" and layer == self.NLAY:
					value = self.parseData.getFloatNum (param[1])
					format = self.parseData.getFloatFormat (param[1])
					if nParams == 6:
						tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					else:
						tmp = "%6.3f" % data[0][0][layer-1]
					param[1] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']				
					line = line.replace(value, tmp)
					
					spaceCount = counts[1:len(counts)]
					line = self.insert_space(9) + self.insertData(param, spaceCount)+"\n"

				elif mask == "fisssi" and ( nextMask == "fisssi" or nextMask == "ifisssi"):
					value = self.parseData.getFloatNum (param[0])
					format = self.parseData.getFloatFormat (param[0])
					
					if nParams == 6:
						tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					else:
						tmp = "%6.3f" % data[0][0][layer-1]
					param[0] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']
					line = line.replace(value, tmp)
					spaceCount = counts[1:len(counts)]
					
					line = self.insert_space(9) + self.insertData(param, spaceCount) +"\n"
					
				elif mask == "fisssi" and nextMask == "ffffffffff":
					print >> output, line,
					line, skipLines = self.getLayerParam(data,layer-1)
					cl = 1	
					while cl < skipLines + 1:
						fileData[l+cl][1] = ""
						cl +=1
					line = ""
	
				elif mask == "fisssi" and layer == self.NLAY:
				
					value = self.parseData.getFloatNum (param[0])
					format = self.parseData.getFloatFormat (param[0])
					
					if nParams == 6:
						tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					else:
						tmp = "%6.3f" % data[0][0][layer-1]
					param[0] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']
					line = line.replace(value, tmp)
					spaceCount = counts[1:len(counts)]
					
					line = self.insert_space(9) + self.insertData(param, spaceCount) +"\n"
				elif mask == "fissi" and ( nextMask == "fissi" or nextMask == "ifisssi"):
					
					value = self.parseData.getFloatNum (param[0])
					format = self.parseData.getFloatFormat (param[0])
					tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					param[0] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']
					line = line.replace(value, tmp)
					spaceCount = counts[1:len(counts)]

				elif mask == "fissi" and layer == self.NLAY:
					value = self.parseData.getFloatNum (param[0])
					format = self.parseData.getFloatFormat (param[0])					
					tmp = "0"+ self.parseData.eformat(data[0][0][layer-1], 2, 4)
					param[0] = tmp + format
					counts = [(len(list(cpart))) for c,cpart in groupby(line) if c == ' ']
					line = line.replace(value, tmp)
					spaceCount = counts[1:len(counts)]

			if line !="":			
				print >> output, line,
		    

		output.close()	
		
    
	def readHDS(self,filename):
	    
	    f = open(filename, "rb")
	    count = 0
	    size = 4+4+4+4+16+4+4+4
	    
	    try:
		bytes ="s"
		lhead = {}
		
		while True:
		    
		    bytes = f.read(size)
		    if bytes == "":
			break
			
		    kstp, kper, pertim, totim, text,nc,nr,ilay= struct.unpack("iiff16siii",bytes)
		    #print kstp, kper,pertim, totim, text, nc,nr,ilay
		    if lhead.has_key(totim) == 0:
			lhead[totim] = numpy.zeros((self.NCOL,self.NROW,self.NLAY), dtype=c_float, order='C')
			
		    for jj in range(nr):
				for ii in range(nc): 
					dat, = struct.unpack("f",f.read(4))
					kk = ilay-1
					lhead[totim][ii,jj,kk]=dat
		    #print ""
		    
	    finally:
		f.close()
	    
	    return lhead
		
	def readUCN(self,filename):
	    
	    f = open(filename, "rb")
	    count = 0
	    size = 4+4+4+4+16+4+4+4
	    
	    try:
		bytes ="s"
		lconcentrations = {}
		
		while True:
		    
		    bytes = f.read(size)
		    if bytes == "":
			break			
		    ntrans, kstp,kper, totim, text,nc,nr,ilay= struct.unpack("iiif16siii",bytes)
		    #print ntrans,kstp, kper,totim, text, nc,nr,ilay
		    if lconcentrations.has_key(totim) == 0:
				lconcentrations[totim] = numpy.zeros((self.NCOL,self.NROW,self.NLAY),dtype=c_float, order='C')
			
		    for jj in range(nr):
				for ii in range(nc): 
					fm = f.read(4)
					dat, = struct.unpack("f",fm)
					kk = ilay-1
					lconcentrations[totim][ii,jj,kk]=dat
	    finally:
		f.close()
	    
	    return lconcentrations
		
	def runSeawat(self):
	    cmd = "cd "+self.modelOutputPath+" && " +self.exePath + " " +self.namFile;
	    p = Popen(cmd , shell=True, stdout=PIPE, stderr=PIPE)
	    out, err = p.communicate()
	    simOutput =  out.rstrip().split('\n')
	    for l in range(len(simOutput)):
		if re.match("(.*)Elapsed run time(.*)", simOutput[l]):
		    self.simTime = float( self.parseData.getSimTime(simOutput[l]) )
