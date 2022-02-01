"""
Script for the EDF simulation
"""

from main_scripts.task_generator import *
import numpy as np


def compute(n_tasks, max_util, times):
    positive_results = 0	# Number of task sets that pass the schedulability condition
    negative_results = 0	# Number of task sets that DO NOT pass the schedulability condition

    for i in range (0, times):

        periods  = np.random.randint(50, 1000, n_tasks)	# Generate the random periods of the tasks
        K = 4   					# Max number of crit_levels (for required failure rates 10^−3,10^−5,10^−7,10^−9)
        perc_WCET = [1, 2, 2, 3]			# For the given fault probability (10^−3/h, 10^−4/h, 10^−5/h), we need 1 re-execution 
        						# in the 10^-3 case, 2 re-execution in the 10^−5 10^−7 cases, and 3 re-execution in the
        						# 10^-9 cases.

	# Randomly assign the criticality levels to the tasks
        task_crit_levels = np.random.randint(1, K+1, n_tasks)

        U = []

        m=0
	# Generate a new random taskset
        scenarios = gen_tasksets(gen_uunifastdiscard(1, max_util, n_tasks), [periods])
        for (c,p) in scenarios[0]:
            U.append(perc_WCET[task_crit_levels[m]-1] * c/p)	# Append utilizations...
            m = m+1

        sched = True

        if sum(U) > 1:		# EDF schedulability condition
            sched = False	# non schedulable

        if sched:
            positive_results += 1
        else:
            negative_results += 1

    return [n_tasks, max_util, positive_results]


