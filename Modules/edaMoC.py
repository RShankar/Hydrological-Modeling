import numpy
import ctypes
from ctypes import *

class edaMoC:

	def caseOne(self, model, lookupTable, inp):

		h     = lookupTable['CLUT']
		hf    = lookupTable['HfLUT']
		diff  = lookupTable['diffLUT']
		col   = 0
		row   = 0
		time  = model.PERLEN
		for ly in range (0, model.NLAY):
			#All the values in the layer are equal
			input =  model.initHead[col][row][ly]
			if input != 0:
				h[time] += input *( hf[ly][time] + diff[ly][time])
				
		return h
		
	def henry( self, model, lookupTable, input):
		polyReg       	= lambda p,i: numpy.polyval(p, i)	
		h 	        = {}
		MoC 		= lookupTable['MoC1']
		h[model.PERLEN] = polyReg(MoC['P'].T, input)	
		return h
		
	def elder( self, model, lookupTable, input):
	
		polyReg       		= lambda p, time: numpy.polyval(p, time)
		MoC 			= lookupTable['MoC1']
		coeffTbl 		= MoC['coeffTbl']
		start 			= model.PERLEN / model.NSTP
		end   			= model.PERLEN 
		t     			= numpy.arange(start, model.PERLEN + start, start)
		h  			= {}
		
		for time in t:
			h[time]  = polyReg(coeffTbl[str(input)].T, time)

		return h