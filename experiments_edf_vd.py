from task_generator import *
from mc_sched import *
import numpy as np
import sys

np.random.seed(12345)

def compute(n_tasks, max_util, times):
    positive_results = 0
    negative_results = 0

    for i in range (0,times):
        periods  = np.random.randint(50, 1000, n_tasks);
        K = 3   # Number of crit_levels
        perc_WCET = [1, 2, 3]

        task_crit_levels = np.random.randint(1, K+1, n_tasks)
        task_crit_levels = [ max(x,1) for x in task_crit_levels ]

        U = []

        m=0
        scenarios = gen_tasksets(gen_uunifastdiscard(1, max_util, n_tasks), [periods])
        for (c,p) in scenarios[0]:

            this_task_U = []
            for j in range(0,task_crit_levels[m]):
                this_task_U.append(perc_WCET[j] * c/p)
            U.append(this_task_U)
            m = m+1

        # Correct the task set to avoid empty levels
        for i in range(1,K):
            num_per_level = 0
            for j in range(0, len(task_crit_levels)):
                if task_crit_levels[j] == i:
                    num_per_level += 1
            if num_per_level==0:
                K -= 1
                for j in range(0, len(task_crit_levels)):
                    if task_crit_levels[j] > i:
                        task_crit_levels[j] = task_crit_levels[j] - 1
                        del U[j][i-1]

        sched = False
        if basic_check(n_tasks, task_crit_levels, U, K):
            sched = True
        if not sched:
            for k in range(1,K):
                if condition_1(k, n_tasks, task_crit_levels, U) and condition_2(k, K, n_tasks, task_crit_levels, U):
                    sched = True
                    break

        if sched:
            positive_results += 1
        else:
            negative_results += 1

    return [n_tasks, max_util, positive_results]

