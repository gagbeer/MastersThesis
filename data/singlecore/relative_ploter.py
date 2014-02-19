#!/usr/bin/env python
import speed
import accuracy
import numpy as np
import numpy as np

import matplotlib 
matplotlib.use('pgf')
import matplotlib.pyplot as plt
#import mpl_toolkits.axisartist as axisartist

font_size = 8
fig_width = 3.0

font = {'family':'serif',
	'serif':['Times'],
	'size':font_size}

plt.rc('text', usetex=True)
plt.rc('font', family='serif')

speed_data = speed.Speed()
accuracy_data = accuracy.Accuracy()

speed_averages = {}
speed_per_mat_element = {}
speed_per_nonzero_element = {}
accuracy_averages = {}
accuracy_per_mat_element = {}

series_to_rowcount = {}
series_to_rowcount[1000.0] = 3000.0
series_to_rowcount[2000.0] = 5400.0
series_to_rowcount[3000.0] = 8526.0
series_to_rowcount[4000.0] = 12288.0
series_to_rowcount[6000.0] = 18468.0
series_to_rowcount[8000.0] = 24000.0
series_to_rowcount[12000.0] = 33396.0
series_to_rowcount[16000.0] = 50700.0
series_to_rowcount[32000.0] = 96768.0

series_to_nonzero_entries = {}
series_to_nonzero_entries[1000.0] = 1994454.0 
series_to_nonzero_entries[2000.0] = 4252230.0
series_to_nonzero_entries[3000.0] = 7792128.0
series_to_nonzero_entries[4000.0] = 15389352.0
series_to_nonzero_entries[6000.0] = 20027466.0
series_to_nonzero_entries[8000.0] = 31388724.0
series_to_nonzero_entries[12000.0] = 40060026.0
series_to_nonzero_entries[16000.0] = 83778480.0
series_to_nonzero_entries[32000.0] = 165140640.0

xlimit = 100000.0
xmin = 2000.0


marker = ""
c =""
"""
fig1= plt.figure(1)
ax1 = fig1.add_subplot(axisartist.Subplot(fig1, "111"))
"""
print speed_data.master_data_map
for subkey in speed_data.master_data_map[speed_data.master_data_map.keys()[0]]:
	speed_averages[subkey] = [[],[],[]]
	speed_per_mat_element[subkey] = []
	speed_per_nonzero_element[subkey] = []
	accuracy_averages[subkey] = [[],[]]
	accuracy_per_mat_element[subkey] = []

for index,key in enumerate(speed_data.master_data_map.keys()):
	print "The key is: " + str(key)
	for subindex, subkey in enumerate(speed_data.master_data_map[key]):
		print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in speed_data.master_data_map[key][subkey]]
		speed_averages[subkey][1].append(np.average(np.copy(subdata)))
		speed_averages[subkey][0].append(series_to_nonzero_entries[float(key)])
		speed_averages[subkey][2].append(series_to_rowcount[float(key)]**2.0)
		speed_per_mat_element[subkey].append((speed_averages[subkey][1][-1]/speed_averages[subkey][2][-1]))
		speed_per_nonzero_element[subkey].append((speed_averages[subkey][1][-1]/speed_averages[subkey][0][-1]))

		


for index, key in enumerate(accuracy_data.master_data_map.keys()):
	print "The key is: " + str(key)
	for subindex, subkey in enumerate(accuracy_data.master_data_map[key]):
		print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in accuracy_data.master_data_map[key][subkey]]
		accuracy_averages[subkey][1].append(np.average(np.copy(subdata)))
		accuracy_averages[subkey][0].append(series_to_nonzero_entries[float(key)])
		accuracy_per_mat_element[subkey].append((accuracy_averages[subkey][1][-1]/accuracy_averages[subkey][0][-1]))

print speed_averages
print accuracy_per_mat_element
"""
cs = ax1.scatter(speed_averages['CS'][2][:], speed_per_mat_element['CS'][:], marker='^', c='g', s=64)
cd = ax1.scatter(speed_averages['CD'][2][:], speed_per_mat_element['CD'][:], marker='x', c='r', s=64 )
fd = ax1.scatter(speed_averages['FD'][2][:], speed_per_mat_element['FD'][:], marker='s', c='b', s=64) 
ad = ax1.scatter(speed_averages['AD'][2][:], speed_per_mat_element['AD'][:], marker='o', c='y', s=64)
plt.xlim(7.e6, 2.e+10)
plt.ylim(0.0, 4.e-7)
plt.xlabel("(log scale) all elements in Jacobian matrix (matrix elements)")
plt.ylabel("Average time to compute Jacobian matrix\nper each element in Jacobian matrix (seconds/ matrix element)")
#plt.title("Average time to compute Jacobian matrix per matrix element\nvs. number of all elements in Jacobian matrix")
plt.legend([cs, cd, fd, ad],['Complex Step', 'Central Difference', 'Forward Difference', 'Automatic Differentiation'], loc=1, scatterpoints = 1, prop={'size':19})
#plt.yticks(np.arange(-10.0, 300.0, 20))
#plt.xticks(np.arange(xmin, xlimit, 4000))
plt.xscale('log')
#plt.yscale('log')
#ax1.xaxis.set_major_formatter(plt.ScalarFormatter())
#ax1.xaxis.set_major_locator(plt.FixedLocator([series_to_nonzero_entries[entry] for entry in [1000, 2000,3000,4000,6000,8000,12000,16000,32000]]))

#ax1.xaxis.major.formatter.set_powerlimits((0,0))
#ax1.axis["bottom"].major_ticklabels.set_axis_direction('left')
ax1.yaxis.set_major_formatter(plt.ScalarFormatter())
ax1.yaxis.major.formatter.set_powerlimits((0,0))
plt.grid(True, which = "both")
plt.grid(True)
ax1.grid(which='major', linestyle='--', color='grey')
"""


fig2= plt.figure(2, figsize=(fig_width*3.5/3.0,fig_width))
ax2 = fig2.add_subplot(111, aspect=4)

cs2= ax2.scatter(speed_averages['CS'][0][:], speed_per_nonzero_element['CS'][:], marker='^', c='g', s=24)
cd2= ax2.scatter(speed_averages['CD'][0][:], speed_per_nonzero_element['CD'][:], marker='x', c='r', s=24 )
fd2= ax2.scatter(speed_averages['FD'][0][:], speed_per_nonzero_element['FD'][:], marker='s', c='b', s=24) 
ad2= ax2.scatter(speed_averages['AD'][0][:], speed_per_nonzero_element['AD'][:], marker='o', c='y', s=24) 
plt.xlim(1.e5, 1.e9)
plt.ylim(0.0, 2.E-6)
plt.xlabel("Number of TS elements", fontsize=font_size)
plt.ylabel("Compute time of TS per element ($\\times 10^{-6}$)", fontsize=font_size)
plt.xscale('log')
plt.grid(True)
ax2.grid(which='major', linestyle='--', color='grey')

plt.yticks([ .5E-6, 1.E-6, 1.5E-6, 2.E-6] ,["$0.5$", "$1.0$","$1.5$", "$2.0$"], fontsize=font_size)
plt.xticks(fontsize=font_size)
plt.legend([cs2, cd2, fd2, ad2],['CS', 'CD', 'FD', 'AD'], scatterpoints = 1, loc='lower left',
                  fancybox=True, shadow=True, ncol=2, fontsize=font_size)



fig2.savefig("serial_speed_rel.pgf")

plt.show()
