#!/usr/bin/env python

import numpy as np

level_1 = np.ones((10,10), dtype=np.longdouble)*2.0 
level_1_norm = np.linalg.norm(level_1)
print level_1_norm

level_1 - np.ones((1*np.sqrt(10.0)

level_2 = np.ones((100,100), dtype=np.longdouble)*(np.divide(level_1_norm, level_1.size))
level_2_norm = np.linalg.norm(level_2)
print level_2_norm

level_3 = np.ones((1000,1000), dtype=np.longdouble)*(np.divide(level_1_norm, np.power(level_1.size,1.5)))
level_3_norm = np.linalg.norm(level_3)
print level_3_norm

level_4 = np.ones((10000,10000), dtype=np.longdouble)*(np.divide(level_1_norm, np.power(level_1.size,2.0)))
level_4_norm = np.linalg.norm(level_4)
print level_4_norm

level_1 += np.random.normal(scale = .001, size=(level_1.shape[0], level_1.shape[1]))
level_2 += np.random.normal(scale = .001, size=(level_2.shape[0], level_2.shape[1]))
level_3 += np.random.normal(scale = .001, size=(level_3.shape[0], level_3.shape[1]))
level_4 += np.random.normal(scale = .001, size=(level_4.shape[0], level_4.shape[1]))

level_1_norm = np.linalg.norm(level_1)
level_2_norm = np.linalg.norm(level_2)
level_3_norm = np.linalg.norm(level_3)
level_4_norm = np.linalg.norm(level_4)

print level_1_norm
print level_2_norm
print level_3_norm
print level_4_norm
