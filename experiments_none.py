from task_generator import *
import numpy as np


def compute(n_tasks, max_util, times):
    positive_results = 0
    negative_results = 0

    for i in range (0, times):

        periods  = np.random.randint(50, 1000, n_tasks);
        K = 3   # Number of crit_levels
        perc_WCET = [1, 2, 3]

        task_crit_levels = np.random.randint(1, K+1, n_tasks)
        task_crit_levels = [ max(x,1) for x in task_crit_levels ]

        U = []

        m=0
        scenarios = gen_tasksets(gen_uunifastdiscard(1, max_util, n_tasks), [periods])
        for (c,p) in scenarios[0]:
            U.append(perc_WCET[task_crit_levels[m]-1] * c/p)
            m = m+1

        sched = True

        if sum(U) > 1:
            sched = False

        if sched:
            positive_results += 1
        else:
            negative_results += 1

    return [n_tasks, max_util, positive_results]


