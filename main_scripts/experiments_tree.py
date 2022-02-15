from main_scripts.task_generator import *
import numpy as np
import math, os, sys
from subprocess import Popen, PIPE
import concurrent.futures

def prepare(id_run, n_tasks, max_util, times, p_fault):

    # Commpute the fault probability
    p_faults = p_fault
    p_faults_per_ms = 1-(1-p_faults)**(1./3600000.)	# Scale down to fault per ms

    positive_results = 0
    negative_results = 0
    
    for i in range(0, times):
        periods  = np.random.randint(50, 1000, n_tasks);
        K = 3   # Number of crit_levels
        crit_lvl_prob = [1e-3, 1e-5, 1e-7, 1e-9]
        perc_WCET = [1, 2, 3]

        task_crit_levels_orig = np.random.randint(0, K+1, n_tasks)
        task_crit_levels = [ max(x,1) for x in task_crit_levels_orig ]

        scenarios = gen_tasksets(gen_uunifastdiscard(1, max_util, n_tasks), [periods])

        strfilename = "/tmp/sec_input_" + str(id_run) + "_" + str(i) + ".txt"
        
        f = open(strfilename, "w")
        
        t_id = 1
        
        for (c,p) in scenarios[0]:
            temp_prob_job = 1-(1-p_faults_per_ms)**p    # Failure prob per job
            temp_prob = 1 - (1-temp_prob_job)**(3600000./p) # Failure prob per hour
                        

            n_reexec_tasks_plus_1 = math.ceil(math.log(crit_lvl_prob[task_crit_levels_orig[t_id-1]-1],temp_prob))

            # WCET, T, N_REEXEC, FAILURE_REQ, FAULT_PROB
            f.write(str(c) + " " + 
                  str(p) + " " +
                  str(n_reexec_tasks_plus_1-1) + " " +
                  str(crit_lvl_prob[task_crit_levels_orig[t_id-1]-1]) + " " +
                  str(temp_prob) + "\n")

            t_id += 1

        f.close()
        
        process = Popen(["./cpp_opt/verify"+str(n_tasks), strfilename], stdout=PIPE)
        (output, err) = process.communicate()
        exit_code = process.wait()
        
        os.remove(strfilename)
        
        if exit_code != 0:
            positive_results = positive_results + 1
		
    return positive_results

def compute(n_tasks, max_util, times, my_id, p_fault):

    positive_results = prepare(my_id, n_tasks, max_util, times, p_fault)
    

    return [n_tasks, max_util, positive_results]
