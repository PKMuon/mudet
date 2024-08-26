#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

data = np.array(open('run.txt').read().strip().split(), dtype='float') + 500
mean, std = np.mean(data), np.std(data)
plt.hist(data, bins=50, histtype='step', color='black')
plt.hist([], bins=50, histtype='step', color='black', label=f'mean={mean:.3f}')
plt.hist([], bins=50, histtype='step', color='black', label=f'std={std:.3f}')
plt.xlabel('Z [mm]')
plt.ylabel('Events')
plt.legend()
plt.grid()
plt.tight_layout()
plt.savefig('ana.pdf')
