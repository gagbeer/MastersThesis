#!/usr/bin/env python
import speed_multi_right as speed_multi	
import accuracy_multi
import numpy as np

import matplotlib 
matplotlib.use('pgf')
import matplotlib.pyplot as plt

font_size = 8
fig_width = 3.0

font = {'family':'serif',
	'serif':['Times'],
	'size':font_size}

plt.rc('text', usetex=True)
plt.rc('font', family='serif')

speed_data = speed_multi.Speed()
accuracy_data = accuracy_multi.Accuracy()

speed_averages = {}
accuracy_averages = {}
accuracy_per_mat_element = {}

marker = ""
c =""
fig1= plt.figure(1, figsize=(fig_width*3.5/3.0,fig_width))
ax1 = fig1.add_subplot(111)

print speed_data.master_data_map
for subkey in speed_data.master_data_map[speed_data.master_data_map.keys()[0]]:
	speed_averages[subkey] = [[],[]]
	accuracy_averages[subkey] = [[],[]]

for index,key in enumerate(speed_data.master_data_map.keys()):
	print "The key is: " + str(key)
	for subindex, subkey in enumerate(speed_data.master_data_map[key]):
		print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in speed_data.master_data_map[key][subkey]]
		speed_averages[subkey][1].append(np.average(np.copy(subdata)))
		speed_averages[subkey][0].append(float(key.split("_")[1]))


for index, key in enumerate(accuracy_data.master_data_map.keys()):
	print "The key is: " + str(key)
	for subindex, subkey in enumerate(accuracy_data.master_data_map[key]):
		print "The subkey is: " + str(subkey)
		subdata = [float(entry) for entry in accuracy_data.master_data_map[key][subkey]]
		accuracy_averages[subkey][1].append(np.average(np.copy(subdata)))
		accuracy_averages[subkey][0].append(float(key.split("_")[1]))

print speed_averages
print accuracy_averages
cs = ax1.scatter(speed_averages['CS'][0][:], speed_averages['CS'][1][:], marker='^', c='g', s=24)
cd = ax1.scatter(speed_averages['CD'][0][:], speed_averages['CD'][1][:], marker='x', c='r', s=24 )
fd = ax1.scatter(speed_averages['FD'][0][:], speed_averages['FD'][1][:], marker='s', c='b', s=24) 
ad = ax1.scatter(speed_averages['AD'][0][:], speed_averages['AD'][1][:], marker='o', c='y', s=24)
#ax1.set_yscale('log')
plt.xlim(24.0, 160.0)
plt.ylim(0.0, 350.0)
plt.xlabel("Number of cores at 1TB RAM per 32 cores", fontsize=font_size)
plt.ylabel("Avg. time to compute TS (s)", fontsize=font_size)
#plt.title("Average time to compute a Jacobian matrix vs. number of cores")
plt.legend([cs, cd, fd, ad],['CS', 'CD', 'FD', 'AD'], scatterpoints = 1, loc='upper right',
                  fancybox=True, shadow=True, ncol=1, fontsize=font_size)

plt.yticks([70.0, 140.0, 210.0, 280.0, 350.0], [70, 140, 210, 280, 350], fontsize=font_size)
plt.xticks(np.arange(0.0,160.0, 32), fontsize=font_size)
plt.grid(True)
ax1.grid(which='major', linestyle='--', color='grey')
fig1.savefig("multi_speed.pgf")

fig2= plt.figure(2, figsize=(fig_width*3.5/3.0,fig_width))
ax2 = fig2.add_subplot(111)
cs2= ax2.scatter(accuracy_averages['CS'][0][:], accuracy_averages['CS'][1][:], marker='^', c='g', s=24)
cd2= ax2.scatter(accuracy_averages['CD'][0][:], accuracy_averages['CD'][1][:], marker='x', c='r', s=24 )
fd2= ax2.scatter(accuracy_averages['FD'][0][:], accuracy_averages['FD'][1][:], marker='s', c='b', s=24) 
plt.xlim(24.0, 160.0)
#plt.ylim(1.e-10, 1.0)
plt.yscale('log')
plt.xlabel("Number of cores at 1TB RAM per 32 cores", fontsize=font_size)
plt.ylabel("$l^2$ norm of difference to AD TS (MPa)", fontsize=font_size)
#plt.title("Frobenius norm of difference to A.D. Jacobian vs. number of cores")
plt.legend([cs2, cd2, fd2],['CS', 'CD', 'FD'], scatterpoints = 1, loc='center left',
                  fancybox=True, shadow=True, ncol=1, fontsize=font_size)


plt.xticks(np.arange(0.0, 160.0, 32), fontsize=font_size)
plt.yticks([1.E-10, 1.E-7, 1.E-4, 1.E-1, 1.E2], ["$10^{-10}$", "$10^{-7}$", "$10^{-4}$", "$10^{-1}$", "$10^{2}$"], fontsize=font_size)
plt.grid(True)
ax2.grid(which='major', linestyle='--', color='grey')

fig2.savefig("multi_accuracy.pgf")
plt.show()
