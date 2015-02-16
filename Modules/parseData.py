import re
import numpy
import re
"""
Parse data module
"""	
class parseData:

	def isFloatNum ( self, num):
	    if re.match("(.*)\.(.*)", num):
		return 1
	    return 0

	def getFloatNum (self,num):
	    n = num.split('(')
	    return n[0]

	def getFloatFormat (self,num):
	    n = num.split('(')
	    
	    return '(' + n[1]

	def isIntNum(self,s):
		return re.match(r"[-+]?\d+$", s) is not None
    
	def eformat(self,f, exp_digits, prec):
		s = "%.*e"%(prec, f)
		mantissa, exp = s.split('e')
		return "%se%+0*d"%(mantissa, exp_digits+1, int(exp)) 
	def getSimTime( self, str):
		t = str.split()
		for i in range(len(t)):
		    if re.match("(.*)\.(.*)", t[i]):
			return float(t[i])
		return -1.0
		
	def getMask (self, str):
		mask = ""
		param = str.split()
		nParams = len(param)
		for i in range(nParams): 
			if self.isIntNum (param[i]) == True:
				mask =  mask + "i"
			elif self.isFloatNum(param[i]) == 1:
				mask =  mask + "f"
			else:
				mask =  mask + "s"
		return mask, nParams, param
