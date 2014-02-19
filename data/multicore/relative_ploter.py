#!/usr/bin/env python
import speed_multi	
import accuracy_multi
import numpy as np

import matplotlib 
matplotlib.use('pgf')
import matplotlib.pyplot as plt

font_size = 8
fig_width = 3.0

font = {'family':'serif',
	'serif':['Times'],
	'size':font_size }

plt.rc('text', usetex=True)
plt.rc('font', family='serif')

speed_data = speed_multi.Speed()
accuracy_data = accuracy_multi.Accuracy()

speed_averages = {}
speed_per_nonzero_element = {}
speed_per_mat_element = {}
accuracy_averages = {}

series_to_nonzero_elements = {}
series_to_nonzero_elements[32] = 1666968710.0
series_to_nonzero_elements[64] = 1666962752.0
series_to_nonzero_elements[96] = 1666760396.0
series_to_nonzero_elements[128] = 1666892660.0

matrix_elements_for_million_element_problem = (3.0*1000000.0)**2.0

marker = ""
c =""
"""
fig1= plt.figure(1)
ax1 = fig1.add_subplot(111)
"""

print speed_data.master_data_map
for subkey in speed_data.master_data_map[speed_data.master_data_map.keys()[0]]:
	speed_averages[subkey] = [[],[],[]]
	speed_per_mat_element[subkey] = []
	speed_per_nonzero_element[subkey] = []
	accuracy_averages[subkey] = [[],[]]

print "Master data map is: ", speed_data.master_data_map
for index,key in enumerate(speed_data.master_data_map.keys()):
	print "The key is: " + str(key)
	for subindex, subkey in enumerate(speed_data.master_data_map[key]):
		print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in speed_data.master_data_map[key][subkey]]
		speed_averages[subkey][1].append(np.average(np.copy(subdata)))
		speed_averages[subkey][0].append(series_to_nonzero_elements[float(key.split("_")[1])])
		speed_averages[subkey][2].append(float(key.split("_")[1]))
		speed_per_mat_element[subkey].append(float(key.split("_")[1])*speed_averages[subkey][1][-1]/matrix_elements_for_million_element_problem)
		speed_per_nonzero_element[subkey].append(float(key.split("_")[1])*speed_averages[subkey][1][-1]/speed_averages[subkey][0][-1])
		print speed_per_mat_element[subkey]

print "Speed averages row 2 is: ", speed_averages
print "Speed per mat element is: ", speed_per_mat_element
print "Speed per nonzero element is: ", speed_per_nonzero_element

for index, key in enumerate(accuracy_data.master_data_map.keys()):
	#print "The key is: " + str(key)
	for subindex, subkey in enumerate(accuracy_data.master_data_map[key]):
		#print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in accuracy_data.master_data_map[key][subkey]]
		accuracy_averages[subkey][1].append(np.average(np.copy(subdata)))
		accuracy_averages[subkey][0].append(float(key.split("_")[1]))
"""
cs = ax1.scatter(speed_averages['CS'][2][:], speed_per_mat_element['CS'][:], marker='^', c='g', s=24)
cd = ax1.scatter(speed_averages['CD'][2][:], speed_per_mat_element['CD'][:], marker='x', c='r', s=24 )
fd = ax1.scatter(speed_averages['FD'][2][:], speed_per_mat_element['FD'][:], marker='s', c='b', s=24) 
ad = ax1.scatter(speed_averages['AD'][2][:], speed_per_mat_element['AD'][:], marker='o', c='y', s=24)
#ax1.set_yscale('log')
plt.xlim(24.0, 136.0)
plt.ylim(5.e-10, 1.3e-9)
plt.xlabel("Number of cores @ 1TB shared memory per 32 cores")
plt.ylabel("Number of cores times average time to compute a\nJacobian matrix per single matrix element\n(cores*seconds per matrix element)")
#plt.title("Number of cores times average time to compute a\nJacobian matrix per single matrix element vs. number of cores")
plt.legend([cs, cd, fd, ad],['Complex Step', 'Central Difference', 'Forward Difference', 'Automatic Differentiation'], loc=4, scatterpoints = 1, prop = {'size':19})
#plt.yticks(np.arange(0.0, 350.0, 20))
plt.xticks(np.arange(24.0,136.0, 8))
plt.grid(True)
"""


fig2= plt.figure(2, figsize=(fig_width*3.5/3.0,fig_width))
ax2 = fig2.add_subplot(111)
cs2= ax2.scatter(speed_averages['CS'][2][:], speed_per_nonzero_element['CS'][:], marker='^', c='g', s=24)
cd2= ax2.scatter(speed_averages['CD'][2][:], speed_per_nonzero_element['CD'][:], marker='x', c='r', s=24 )
fd2= ax2.scatter(speed_averages['FD'][2][:], speed_per_nonzero_element['FD'][:], marker='s', c='b', s=24) 
ad2= ax2.scatter(speed_averages['AD'][2][:], speed_per_nonzero_element['AD'][:], marker='o', c='y', s=24)
plt.xlim(24.0, 160.0)
plt.ylim(3.0e-6, 8.e-6)
plt.xlabel("Cores at 1TB RAM per 32 cores", fontsize=font_size)
plt.ylabel("Cores / compute time per TS element ($\\times 10^{-6}$/s)", fontsize=font_size)
#plt.title("Number of cores times average time to compute a Jacobian \nmatrix per single nonzero matrix element vs. number of cores")
plt.legend([cs2, cd2, fd2, ad2],['CS', 'CD', 'FD', 'AD'], scatterpoints = 1, loc='upper left', bbox_to_anchor=(0., 1.),
                  fancybox=True, shadow=True, ncol=2, fontsize=font_size)

plt.xticks(np.arange(0.0, 160.0, 32), fontsize=font_size)
plt.yticks([4.E-6, 5.E-6, 6.E-6, 7.E-6, 8.E-6], ["$4$", "$5$", "$6$", "$7$", "$8$"], fontsize=font_size)
plt.grid(True)
ax2.grid(which='major', linestyle='--', color='grey')

fig2.savefig("multi_speed_rel.pgf")

plt.show()
