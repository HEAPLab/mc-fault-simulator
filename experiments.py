# Main simulator file
import sys
import experiments_none
import experiments_edf_vd
import experiments_edf_vd_old
import numpy as np
from multiprocessing import Pool
import queue

np.random.seed(12345)   # This allows us to be 

MIN_TASKS = 1
MAX_TASKS = 50
MIN_UTIL  = 1   # In percentage
MAX_UTIL  = 100 # In percentage
N_RUNS    = 1000
PARALLEL  = 8

if len(sys.argv) == 3:
    MIN_TASKS = int(sys.argv[1])
    MAX_TASKS = int(sys.argv[1])
    MIN_UTIL  = int(sys.argv[2]) # In percentage
    MAX_UTIL  = int(sys.argv[2]) # In percentage

pool = Pool()
i = 0
q = queue.Queue()

def print_q(q):
    while not q.empty():
        result = q.get()
        output = result.get()
        print(str(output[0]) + " " + str(output[1]) + " " + str(output[2]))

for n_tasks in range (MIN_TASKS, MAX_TASKS+1):
    sys.stderr.write("Progress: " + str(n_tasks)+"/"+str(MAX_TASKS)+"\n")

    for max_util in range (MIN_UTIL, MAX_UTIL+1):
        worker = pool.apply_async(experiments_edf_vd.compute, [n_tasks, max_util/100.0, N_RUNS])
    #    worker = pool.apply_async(experiments_none.compute, [n_tasks, max_util/100.0, N_RUNS])
        q.put(worker)
        i = i + 1
        if i % PARALLEL == 0:
            print_q(q)

    print_q(q)
