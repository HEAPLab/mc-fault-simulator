from main_scripts.task_generator import *
from main_scripts.mc_sched import *
import numpy as np
import sys

np.random.seed(12345)

def compute(n_tasks, max_util, times, p_fault, consider_faults, faults_only):

    p_faults = p_fault # SET HERE
    p_faults_per_ms = 1-(1-p_faults)**(1./3600000.)

    positive_results = 0
    negative_results = 0

    for i in range (0,times):
    
        # Generate random periods between 50 and 1000
        periods  = np.random.randint(50, 1000, n_tasks);
        K = 3   # Number of crit_levels
        perc_WCET = [1, 2, 3]

        # Now assign random criticality levels
        crit_lvl_prob = [1e-3, 1e-5, 1e-7, 1e-9]  # DAL D, B, A
#        task_crit_levels = np.random.randint(1,K+1, n_tasks)
        task_crit_levels_orig = np.random.randint(0,K+1, n_tasks)
#        task_crit_levels = [ min(max(x,1), 2) for x in task_crit_levels_orig ]
        task_crit_levels = [ 2 if x==0 else x for x in task_crit_levels_orig ]

#        task_crit_levels = [ 2 if x==4 else max(x,1) for x in task_crit_levels ]
        U = []

        m=0
        scenarios = gen_tasksets(gen_uunifastdiscard(1, max_util, n_tasks), [periods])
        
        p_to_fault = []
        for (c,p) in scenarios[0]:

            this_task_U = []
            for j in range(0,task_crit_levels[m]):
                this_task_U.append(perc_WCET[j] * c/p)
            U.append(this_task_U)
            m = m+1

            # Compute the probability of failure
            temp_prob_job = 1-(1-p_faults_per_ms)**p    # Failure prob per job
            temp_prob = 1 - (1-temp_prob_job)**(3600000./p) # Failure prob per hour
            
            p_to_fault.append(temp_prob) 


        is_fault_ok = True
        if consider_faults:
            # Compute the cross-interference probability of failure due to a possible mode switch
            p_to_fault_aft = 1 - np.array(p_to_fault)   # Probability of not receiving a fault
            for t in range(0, n_tasks): # IO
                for t2 in range(0, n_tasks): # GLi altri
                    if task_crit_levels[t2] < task_crit_levels[t]:
                        p_to_fault_aft[t2] *= (1-p_to_fault[t])
            p_to_fault_aft = 1-p_to_fault_aft

            # And now add the faul totlerance of the restart jobs
            for t in range(0, n_tasks):
                p_to_fault_aft[t] **= task_crit_levels[t]
            
            
            for t in range(0, n_tasks):
                if p_to_fault_aft[t] > crit_lvl_prob[task_crit_levels[t]-1]:
                    is_fault_ok = False
                    break

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
        
        if faults_only:
            sched = is_fault_ok

        if not faults_only and is_fault_ok:

            # Check schedulability condition (formed by 2 conditions)
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

