from ctypes import *
import ctypes
import numpy
import os
import time
import struct
import math
  
class hydrologicalSimulation:

	def __init__(self, nCol, nRow, nLay):
	
		self.nCol = nCol
		self.nRow = nRow
		self.nLay = nLay
		
	def startSim(self, timeStep, simulationTime):
		
		return 1
		
	def initArray(self, data,nCol,nRow,nLayer,value):	
		for kk in range (0, nLayer):
			for jj in range (0, nRow):
				for ii in range (0, nCol):
					data[ii,jj,kk]=value
					
	def setLayer(self, data,nCol,nRow,layer,value):
		for jj in range (0, nRow):
			for ii in range (0, nCol):
				data[ii,jj,layer]=value
	
	def clearWellData(self, model):
		if os.path.exists(model.welFilePath):	
			for key, value in sorted(model.welData.iteritems()):
					model.welData[key] = 0.0
				
	def changeNonZeroArray(self, data,nCol,nRow,nLayer,value):	
		for kk in range (0, nLayer):
			for jj in range (0, nRow):
				for ii in range (0, nCol):
					if data[ii, jj, kk] != 0:
						data[ii,jj,kk]=value
		
	def writeHDS(self, headOut, filename):
	
		text = ''
		for num in xrange(12):
			text += chr(0x20)
			
		text += "HEAD"
		kstp = 0
		kper = 1
		nc = self.nCol
		nr = self.nRow

		f = open(filename,'wb')		
		for totim, data in sorted(headOut.iteritems()):		
			#print totim
			kstp = kstp + 1
			for kk in range (0, self.nLay):
				pertim = totim
				ilay= kk + 1
				#print kstp, kper,pertim, totim, text, nc,nr,ilay 
				header = struct.pack("iiff16siii",kstp, kper, pertim, totim, text,nc,nr,ilay)
				packed = [struct.pack('f', val) for val in data[:self.nCol,:self.nRow,kk]]

				#f.write(b''.join(header))
				#f.write(b''.join(packed))
				
				f.write(b''.join(bytes(x) for x in header))
				f.write(b''.join(bytes(x) for x in packed))
						
		f.close()

					
			
			