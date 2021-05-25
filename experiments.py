# Main simulator file
import sys
import experiments_none
import experiments_edf_vd
import experiments_tree
import numpy as np
from multiprocessing import Pool
import queue

np.random.seed(12345)   # This allows us to be 

NR_TASKS  = [5, 10, 25, 50] 
MIN_UTIL  = 1   # In percentage
MAX_UTIL  = 100 # In percentage
N_RUNS    = 100
PARALLEL  = 8
FAULT_P   = 1e-3

if len(sys.argv) == 3:
    MIN_UTIL  = int(sys.argv[2]) # In percentage
    MAX_UTIL  = int(sys.argv[2]) # In percentage

pool = Pool(processes=PARALLEL)
i = 0
q = queue.Queue()

def print_q(q):
    while not q.empty():
        result = q.get()
        output = result.get()
        print(str(output[0]) + " " + str(output[1]) + " " + str(output[2]))
        sys.stdout.flush()

for n_tasks in NR_TASKS:
    sys.stderr.write("Queing: " + str(n_tasks)+"/"+str(len(NR_TASKS))+"\n")

    for max_util in range (MIN_UTIL, MAX_UTIL+1):
        if max_util % 5 != 0:
            continue

    #    worker = pool.apply_async(experiments_edf_vd.compute, [n_tasks, max_util/100.0, N_RUNS, FAULT_P, True, False])
    #    worker = pool.apply_async(experiments_none.compute, [n_tasks, max_util/100.0, N_RUNS])
    
        worker = pool.apply_async(experiments_tree.compute, [n_tasks, max_util/100.0, N_RUNS, i, FAULT_P])
        q.put(worker)

        i = i + 1

print_q(q)
