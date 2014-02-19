#!/usr/bin/env python

import numpy as np
import glob

class Accuracy:
	def __init__(self):
		accuracyfilenames = glob.glob("./*_accuracy")

		stripchars = '_.\/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()'

		lines = []
		data = []
		keys = []

		'''
		load all records into data, divided by problem size -> iteration -> calculation type
		'''
		for filename in accuracyfilenames:
			f = open(filename)
			lines = [line.split()[-1] for line in f.readlines()]
			data.append([ [filename.strip(stripchars), line] for line in lines])
		'''
		create a dictionary which holds the index of the problem
		size as found in the data array
		'''
		for problemsize in data:
			keys.append(problemsize[0][0])

		problemsize_dictionary = {}
		for index, key in zip(range(len(keys)),keys):
			problemsize_dictionary[key] = index

		#print("The dictionary is: ", problemsize_dictionary)
		'''
		load the problem size numbers and measurements from the data array into 
		accuracymeasurements
		'''
		accuracymeasurements = []
		for problemsize in data:
			accuracymeasurements.append([])

		for problemsize in data:
			accuracymeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			accuracymeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			accuracymeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			for row in range(len(problemsize)):
				calc_id = (row +1)%3 

				if(calc_id == 0):
					#print('Central Differnce')
					accuracymeasurements[problemsize_dictionary[problemsize[0][0]]][2].append(problemsize[row][1])

				elif(calc_id == 1):
					#print('Complex Step')
					accuracymeasurements[problemsize_dictionary[problemsize[0][0]]][0].append(problemsize[row][1])

				elif(calc_id == 2):
					#print('Forward DIfference')
					accuracymeasurements[problemsize_dictionary[problemsize[0][0]]][1].append(problemsize[row][1])

				else:
					print("calc_id reached an unexpected value, calc_id was: ", calc_id)

		self.master_data_map = {}

		for key in problemsize_dictionary.keys():
			self.master_data_map[key] = dict(CD= np.array(accuracymeasurements[problemsize_dictionary[key]][2]),
					       FD= np.array(accuracymeasurements[problemsize_dictionary[key]][1]),
					       CS= np.array(accuracymeasurements[problemsize_dictionary[key]][0]))

			#print("The keys of self.master_data_map[{key}] are: " +str(self.master_data_map[key].keys())).format(key=key)

			#for subkey in self.master_data_map[key].keys():
			#	print("The values for self.master_data_map[{key}][{subkey}] are: " + str(self.master_data_map[key][subkey])).format(key=key, subkey=subkey)


