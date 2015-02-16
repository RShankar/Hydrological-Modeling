import time
import shutil
import os
import sys
import glob
import cPickle
import numpy
import statsmodels.api as smf

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FormatStrFormatter
from scipy.interpolate import interp1d
from distutils.util import strtobool

class common:

	def __init__(self):
	
		self.resultsPath  = os.path.join(os.getcwd(), 'Results')
		self.benchmarkFilePath  = os.path.join(self.resultsPath, 'benchmarkResults.csv')
		self.f = open(self.benchmarkFilePath, 'w')
		
	
	def init(self, nCol, nRow, nLay):
	
		self.nCol = nCol
		self.nRow = nRow
		self.nLay = nLay
		
		
	def __exit__(self, type, value, traceback):
	
		print "Deinit common"
		close(self.f)
		
	def queryYesOrNoAnswerFromUser(self, question):
	    sys.stdout.write('%s [y/n]\n' % question)
	    while True:
		try:
		    return strtobool(raw_input().lower())
		except ValueError:
		    sys.stdout.write('Please respond with \'y\' or \'n\'.\n')
		    
	def createModelOutputPath(self, modelName, prefix):
		modelOutputPath = os.path.join( os.getcwd(), modelName + prefix)
		
		if not os.path.exists(modelOutputPath):
			os.makedirs(modelOutputPath)
		else:
			shutil.rmtree(modelOutputPath)
			os.makedirs(modelOutputPath)
		return modelOutputPath
		
	def createOutputPath(self, root, dirName):
		path = os.path.join( root, dirName)
		if not os.path.exists(path):
			os.makedirs(path)
		else:
			shutil.rmtree(path)
			os.makedirs(path)
		return path
		
	def storeLUT(self, fileName, table):
	
		f = open(fileName, "wb")
		cPickle.dump(table, f, protocol=2)	
		f.close()
		
	def loadLUT(self, fileName):
	
		table = []
		if os.path.exists(fileName):
			f = open(fileName,"rb")
			table = cPickle.load(f)
			f.close()
		else:
			print "Could not load date from filename", fileName
			
		return table
		
	def sumOfSquaredResiduals(self, a, b, ii, jj, kk):

		x = []
		A = []
		B = []

		x.append(0)
		A.append(0)
		B.append(0)

		for time, data in sorted(a.iteritems()):
			x.append(time)
			
		for time, data in sorted(a.iteritems()):
			A.append(data[ii,jj,kk])
			
		for time, data1 in sorted(b.iteritems()):
			B.append(data1[ii,jj,kk])
			
		stats = smf.OLS(numpy.array(A), numpy.array(B)).fit()
		return stats.rsquared
		
	def sumOfSquaredResHeads(self, original, newModel):
	
		maxError = 0.0
		minError = 100.0
		list = []
		imin = 0
		jmin = 0
		kmin = 0
		
		for jj in range (0, self.nRow):
			for ii in range (0, self.nCol):
				for kk in range (0, self.nLay):	
					rss = self.sumOfSquaredResiduals(original, newModel, ii, jj, kk)
					list.append(rss)			
					if maxError < rss:
						maxError = rss
					if minError > rss:
						minError = rss
						
						imin = ii
						jmin = jj
						kmin = kk
		
		
		hist, bin_edges = numpy.histogram(list, bins = [0.0, 0.95, 1.0])

		
		return maxError, minError, hist,imin,jmin,kmin
		
	def runOriginalModel(self, model):
		elapsedTime =  time.clock()
		model.runSim()
		elapsedTime =  time.clock() - elapsedTime	
		return  model.head, elapsedTime
		
	def runEDAModelWithLUT(self, modelName, model, mocFunction, lutTable, input):
		elapsedTime =  time.clock()
		dataHeads   =  mocFunction(model, lutTable, input)
		elapsedTime =  time.clock() - elapsedTime
		return elapsedTime, dataHeads
	
	def saveHeads(self, hdsFileName, dataHeads, hydroSim):
		hydroSim.writeHDS( dataHeads, hdsFileName)
		
	def compareMoCs(self, modelName, originalModelTime, newModelTime, originalModelHead, dataHeads):
	
		self.speedup = originalModelTime/newModelTime
		self.maxRsquared, self.minRsquared, self.hist, self.imin, self.jmin, self.kmin  = self.sumOfSquaredResHeads(originalModelHead, dataHeads)
		
		#print modelName,",", originalModelTime, ",", newModelTime, ",", speedup, ",", maxRsquared, ",", minRsquared, ",",
		self.totalNumberOfCells = self.hist[0] + self.hist[1]
		#print totalNumberOfCells, ",",
		self.minRsquaredLessThan90Pecent 		= self.hist[0]
		self.minRsquaredGreaterThanOeEqualTo90Pecent 	= self.hist[1]		 
		#print minRsquaredLessThan90Pecent, ",",
		#print minRsquaredGreaterThanOeEqualTo90Pecent
	
	def addColDes(self, inputValueName):
	
		print >> self.f, "",",", 
		print >> self.f, "",",", 		
		print >> self.f, "",",", 
		print >> self.f, "",",",
		print >> self.f, "",",", 
		print >> self.f, "",",", 		
		print >> self.f, "",",", 
		print >> self.f, "",",",
		print >> self.f, "",",", 
		print >> self.f, ""
		
		print >> self.f, "Model Name",",", 
		print >> self.f, inputValueName, ",", 
		print >> self.f, "HYDR Simulation Time", ",", 
		print >> self.f, "EDA Simulation Time", ",", 
		print >> self.f, "Speed Up", ",",
		print >> self.f, "Max Rsquared",",", 
		print >> self.f, "Min Rsquared",",",
		print >> self.f, "Total Number Of Cells" , ",",
		print >> self.f, "Min Rsquared < 90%", ",",
		print >> self.f, "Min Rsquared >= 90%"
		
	def analyzeData(self, inputValue, model, modelName, originalModelTime, originalModelHead, newModelTime, newModelHead):

		#Populate model.NCOL, model.NROW, model.NLAY to the common function object
		self.init(model.NCOL, model.NROW, model.NLAY)

		#Compare between the original model and the new model
		self.compareMoCs(modelName, originalModelTime, newModelTime, originalModelHead, newModelHead)
		print >> self.f, modelName, ",", 
		print >> self.f, inputValue, ",", 
		print >> self.f, originalModelTime, ",", 
		print >> self.f, newModelTime, ",", 
		print >> self.f, self.speedup, ",", 
		print >> self.f, self.maxRsquared, ",", 
		print >> self.f, self.minRsquared, ",",
		print >> self.f, self.totalNumberOfCells, ",",
		print >> self.f, self.minRsquaredLessThan90Pecent, ",",
		print >> self.f, self.minRsquaredGreaterThanOeEqualTo90Pecent
		
		modelResultsPath = os.path.join(self.resultsPath, modelName)		
		
		if not os.path.exists(modelResultsPath):
			modelResultsPath = self.createOutputPath(self.resultsPath, modelName)	
			
		#Save head results (HYDR and EDA) in comman version seperated file
		filePath   = os.path.join(modelResultsPath, str(inputValue)+"_HYDR_"+modelName+".csv")
		self.saveHeadsToFile(filePath, originalModelHead)
		
		filePath   = os.path.join(modelResultsPath, str(inputValue)+"_EDA_"+modelName+".csv")
		self.saveHeadsToFile(filePath, newModelHead)
		
	def saveModel(self, model, modelName, des, root, head, hydroSim):
	
		topPath = os.path.join(os.getcwd(), root)
		
		if not os.path.exists(topPath):
			topPath = self.createOutputPath(os.getcwd(), root)
			
		outputPath = os.path.join(topPath, modelName)
		
		if not os.path.exists(outputPath):
			outputPath = self.createOutputPath(topPath, modelName)
		
		lPath = self.createOutputPath(outputPath, des)
		source_dir = model.modelOutputPath
		for filename in glob.glob(os.path.join(source_dir, '*.*')):
			if filename.find(".hds") == -1:
				shutil.copy(filename, lPath)

				
		modelVieverFile = os.path.join(os.getcwd(), "ModelViewer", modelName + ".mv")
		newFilePath = os.path.join(lPath, modelName +".mv")
		
		shutil.copy(modelVieverFile, newFilePath)
		fileName = os.path.join(lPath, modelName +".hds")
		hydroSim.writeHDS( head, fileName)	
		
	def localMininaMaxima(self, x,y):

		gradients = numpy.diff(y)	
		maxima_num=0
		minima_num=0
		count=0
		
		xMin = []
		yMin = []
		
		xMax = []
		yMax = []
		
		for i in gradients[:-1]:
			count+=1
			if ((cmp(i,0)>0) & (cmp(gradients[count],0)<0) & (i != gradients[count])):
				maxima_num+=1
				xMin.append(x[count])
				yMin.append(y[count])
			if ((cmp(i,0)<0) & (cmp(gradients[count],0)>0) & (i != gradients[count])):
				minima_num+=1
				xMax.append(x[count])
				yMax.append(y[count])
		
		return maxima_num, minima_num, xMin, yMin, xMax,yMax
		
	def getPeaks(self, xi, yi):

		a=numpy.array(yi,dtype=numpy.float)
		
		gradients=numpy.diff(a)
		maxima_num=0
		minima_num=0
		max_locations=[]
		min_locations=[]
		count=0
		
		x = []
		y = []
		x.append(xi[0])
		y.append(yi[0])
		
		for i in gradients[:-1]:
			count+=1
			if ((cmp(i,0)>0) & (cmp(gradients[count],0)<0) & (i != gradients[count])):
				maxima_num+=1
				x.append(xi[count])
				y.append(a[count])
				max_locations.append(count)
				
			if ((cmp(i,0)<0) & (cmp(gradients[count],0)>0) & (i != gradients[count])):
				minima_num+=1
				x.append(xi[count])
				y.append(a[count])	
				min_locations.append(count)
		x.append(xi[len(xi) -1 ])
		y.append(yi[len(xi) -1])
		
		return x,y
		
	def saveHeadsToFile(self, filecsv, head):
		output = open(filecsv, "w")
		for time, value in sorted(head.iteritems()):
			print >> output, "Time,",time
			lay = "Layer,"			
			for kk in range (0, self.nLay):
				lay = lay + str(kk) + ","
				
			print >> output, lay
			print >> output, "";
			for jj in range (0, self.nRow):
				for ii in range (0, self.nCol):
					print >> output, "\"(",jj,",",ii,")\"",",",
					for kk in range (0, self.nLay):
						v = value[ii, jj, kk]
						print >> output, v, ",",
					print >> output, "";			

		output.close()
		
	def plotCellsOverTime(self, modelName, title, model, newModel, ii, jj, kk, rr):

		y = []
		y1 =[]
		x = []
		lx = []
		ly = []
		
		plt.figure()
		plt.title(title, fontsize=28)
		
		params = {'legend.fontsize': 20, 'legend.linewidth': 2}
		plt.rcParams.update(params)
		
		for time, value in sorted(model.iteritems()) :
				x.append(time)
				y.append(value[ii, jj, kk])
				
		
		maxima_num, minima_num, xMin, yMin, xMax,yMax = self.localMininaMaxima(x,y)
		
		a = numpy.column_stack((x,y))
		xmax, ymax = a.max(axis = 0)
		l = plt.axhline(y = ymax, linestyle='-', color='m', label = 'HYDR Global Maxima')
		
		xmin, ymin = a.min(axis = 0)
		l = plt.axhline(y = ymin, linestyle='-', color='y', label = 'HYDR Global Minima')
		
		step  = abs(( max(y)  - min(y) ) / 10.0)
		yt = numpy.arange(min(y) - step, max(y) + step, step)
		
		for degree, data in sorted(newModel.iteritems()):
			y1 = []
			
			for time, value in sorted(data.iteritems()) :
					y1.append(value[ii, jj, kk])
					
			plt.plot(x, y1, '-', label= "EDA Data $R^{2}$=" + "%.2f" % rr[degree] + " Degree %d" % degree )
			
			maxima_num, minima_num, xMin, yMin, xMax,yMax = self.localMininaMaxima(x,y1)
			
			a = numpy.column_stack((x,y1))
			xmax, ymax = a.max(axis = 0)
			yt = numpy.append(yt, ymax)
			
			l = plt.axhline(y = ymax, linestyle='--', color='m', label = 'EDA Global Maxima')
		
			xmin, ymin = a.min(axis = 0)
			l = plt.axhline(y = ymin, linestyle='--', color='y', label = 'EDA Global Minima')
			yt = numpy.append(yt, ymin)
			
		plt.plot(x, y, 'bs', label="HYDR Data")
		
		x1, y1 = self.getPeaks(x,y)
		
		f = interp1d(x1, y1, kind = 'linear')
		#ynew = f(x)
		ynew = numpy.interp(x, x1, y1)
		
		plt.plot(x, ynew, 'r-', label="Linear Interpolation")
		ax = plt.gca()
		
		for t in x:
			l = plt.axvline(x=t, linestyle='--', color='k')

		# recompute the ax.dataLim
		ax.relim()
		
		# update ax.viewLim using the new dataLim
		#ax.autoscale_view()
		plt.draw()
		
		# Shrink current axis's height by 10% on the bottom
		box = ax.get_position()
		ax.set_position([box.x0, box.y0 + box.height * 0.1,
		box.width, box.height * 0.9])
		 
		#Put a legend below current axis
		ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.07), fancybox=True, shadow=True, ncol=3)
		
		#Time labels
		#X axis label
		plt.xlabel('Time (days)', fontsize=24)
		
		#Y axis label
		plt.ylabel('Head (m)',    fontsize=24)
		
		yFormatter = FormatStrFormatter('%e')
		ax.yaxis.set_major_formatter(yFormatter)
		
		#Save plot
		modelResultsPath = os.path.join(self.resultsPath, modelName)		
		if not os.path.exists(modelResultsPath):
			modelResultsPath = self.createOutputPath(self.resultsPath, modelName)
		
		fileName = os.path.join(modelResultsPath, title + '.png')	 	
		fig = plt.gcf()
		fig.set_size_inches(22.5, 13.5)
		fig.savefig(fileName, dpi=300)	
		
		