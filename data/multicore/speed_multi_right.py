#!/usr/bin/env python
#This file is called right because there was a particular correction made.
#rather than taking the running average of the Jacobian calculation speed
#and taking the average of the running average downstream the instant
#calculation time is instead collected and then averaged downstream


import numpy as np
import glob

class Speed:
	def __init__(self):		
		speedfilenames = glob.glob("./*_speed")

		stripchars = '_.\/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()'

		lines = []
		data = []
		keys = []

		'''
		load all records into data, divided by problem size -> iteration -> calculation type
		'''
		for filename in speedfilenames:
			f = open(filename)
			lines = [[entry.strip('()') for entry in line.split()[-6:]] for line in f.readlines()]
			data.append([ [filename.strip(stripchars), line[0], line[1]] for line in lines])
		'''
		create a dictionary which holds the index of the problem
		size as found in the data array
		'''
		for problemsize in data:
			keys.append(problemsize[0][0])

		problemsize_dictionary = {}
		for index, key in zip(range(len(keys)),keys):
			problemsize_dictionary[key] = index

		#print"The dictionary is: ", problemsize_dictionary
		'''
		load the problem size numbers and measurements from the data array into 
		speedmeasurements
		'''
		speedmeasurements = []
		for problemsize in data:
			speedmeasurements.append([])

		for problemsize in data:
			speedmeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			speedmeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			speedmeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			speedmeasurements[problemsize_dictionary[problemsize[0][0]]].append([])
			for row in range(len(problemsize)):
				calc_id = (row +1)%4 

				if(calc_id == 0):
					if row == 3:
						#print('Forward Difference')
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][3].append(float(problemsize[row][1]))
					else:
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][3].append(float(problemsize[row][1]) -  float(problemsize[row-4][1]))

				elif(calc_id == 3):
					if row == 2:
						#print('Complex Step')
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][2].append(float(problemsize[row][1])) 
					else:
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][2].append(float(problemsize[row][1]) -  float(problemsize[row-4][1]))

				elif(calc_id == 2):
					if row == 1:
						#print('Central Difference')
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][1].append(float(problemsize[row][1])) 
					else:
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][1].append(float(problemsize[row][1]) - float(problemsize[row-4][1]))

				elif(calc_id == 1):
					if row == 0:
						#print('Automatic Differentiation')
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][0].append(float(problemsize[row][1]))
					else:
						speedmeasurements[problemsize_dictionary[problemsize[0][0]]][0].append(float(problemsize[row][1]) -float(problemsize[row-4][1]))

				else:
					print("calc_id reached an unexpected value, calc_id was: ", calc_id)

		self.master_data_map = {}

		for key in problemsize_dictionary.keys():
			self.master_data_map[key] = dict(AD= np.array(speedmeasurements[problemsize_dictionary[key]][0]),
					       CD= np.array(speedmeasurements[problemsize_dictionary[key]][1]),
					       CS= np.array(speedmeasurements[problemsize_dictionary[key]][2]),
					       FD= np.array(speedmeasurements[problemsize_dictionary[key]][3]))

			#print("The keys of self.master_data_map[{key}] are: " +str(self.master_data_map[key].keys())).format(key=key)

			#for subkey in self.master_data_map[key].keys():
			#	print("The values for self.master_data_map[{key}][{subkey}] are: " + str(self.master_data_map[key][subkey])).format(key=key, subkey=subkey)

def main():
	import speed_multi_right
	apples = speed_multi_right.Speed()

	print "Master data map is: ", apples.master_data_map

if __name__ == "__main__":
	main()
