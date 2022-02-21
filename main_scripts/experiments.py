# Main simulator file
import sys, os
from pathlib import Path
from main_scripts import experiments_none
from main_scripts import experiments_edf_vd
from main_scripts import experiments_tree
import numpy as np
from multiprocessing import Pool
import queue

total_done = 0
current_outfile = ""
current_outfile_handler = None

def print_q(q, n_steps):
    global total_done, current_outfile
    while not q.empty():
        result = q.get()
        output = result.get()
        current_outfile_handler.write(str(output[0]) + " " + str(output[1]) + " " + str(output[2]) + "\n")
        current_outfile_handler.flush()
        total_done = total_done + 1

        printProgressBar(total_done, n_steps, prefix = 'Progress:', suffix = 'Complete', length = 50)


def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█', printEnd = "\r"):
	"""
	Call in a loop to create terminal progress bar
	@params:
	iteration   - Required  : current iteration (Int)
	total       - Required  : total iterations (Int)
	prefix      - Optional  : prefix string (Str)
	suffix      - Optional  : suffix string (Str)
	decimals    - Optional  : positive number of decimals in percent complete (Int)
	length      - Optional  : character length of bar (Int)
	fill        - Optional  : bar fill character (Str)
	printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
	"""
	percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
	filledLength = int(length * iteration // total)
	bar = fill * filledLength + '-' * (length - filledLength)
	print(f'\r{prefix} |{bar}| {percent}% {suffix}', end = printEnd)
	# Print New Line on Complete
	if iteration == total: 
		print()

def prepare_output(kind, p):
	global current_outfile, current_outfile_handler
	Path("results").mkdir(parents=True, exist_ok=True)
	name = "results_"
	if kind == 1:
		name = name + "edf"
	elif kind == 2:
		name = name + "edf_vd"
	elif kind == 3:
		name = name + "tree"
	current_outfile = "results/" + name + "_" + '{:.0e}'.format(p) + ".txt"

	if current_outfile_handler != None:
		current_outfile_handler.close()
	current_outfile_handler = open(current_outfile, "w")

def run_sim(kind, seed, n_tasks_array, min_util, max_util, n_runs, parallel, fault_p_array):
	global total_done
	pool = Pool(processes=parallel)
	for fault_p in fault_p_array:
		prepare_output(kind, fault_p)
		i = 0
		total_done = 0
		q = queue.Queue()
		np.random.seed(seed)
		if kind != 1:
			print("\nRunning for failure probability: " + '{:.0e}'.format(fault_p))
		n_steps = len(n_tasks_array)*len(range (min_util, max_util+1))/5
		printProgressBar(0, n_steps, prefix = 'Progress:', suffix = 'Complete', length = 50)
		for n_tasks in n_tasks_array:
			for mu in range (min_util, max_util+1):
				if mu % 5 != 0:
					continue

				if kind == 1:
					worker = pool.apply_async(experiments_none.compute, [n_tasks, mu/100.0, n_runs])
				elif kind == 2:
					worker = pool.apply_async(experiments_edf_vd.compute, [n_tasks, mu/100.0, n_runs, fault_p, True, False])
				elif kind == 3:
					worker = pool.apply_async(experiments_tree.compute, [n_tasks, mu/100.0, n_runs, i, fault_p])
				q.put(worker)

				i = i + 1

		print_q(q, n_steps)
		if kind == 1:	# Only 1 execution in case of EDF as the fault probability has no effect
			break
			


