import sys, os, subprocess, time
from main_scripts import experiments
from subprocess import Popen, PIPE

# Some example of yes/no answers
yes = {'yes','y', 'ye', ''}
no = {'no','n'}

# Internal configuration
NR_TASKS  = [5, 10, 25, 50] 
MIN_UTIL  = 1   # In percentage
MAX_UTIL  = 100 # In percentage
FAULT_P   = [1e-3, 1e-4, 1e-5]

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

print("""
    ____ _____  _    ____    ____   ___ ____  ____       _    _____ 
   |  _ \_   _|/ \  / ___|  |___ \ / _ \___ \|___ \     / \  | ____|
   | |_) || | / _ \ \___ \    __) | | | |__) | __) |   / _ \ |  _|  
   |  _ < | |/ ___ \ ___) |  / __/| |_| / __/ / __/   / ___ \| |___ 
   |_| \_\|_/_/   \_\____/  |_____|\___/_____|_____| /_/   \_\_____|
                                                                 
""")
print(f"{bcolors.OKCYAN}                            Welcome!                       ")
print(f"I am the Artifact Evaluation Script for submission no. 15 at RTAS 2022,")
print(f"and I will guide you in testing the scripts used to generate the results.")
print(f"I will not describe you all the script in the details, so please refer to")
print(f"the provided documentation for a complete description of these AE scripts.\n")

print(f"Here some info as a reference:{bcolors.ENDC}")
print(f"{bcolors.BOLD}Paper title{bcolors.ENDC}: 'A Mixed-Criticality Approach to Fault Tolerance: Integrating")
print(f"Schedulability and Failure Requirements'")
print(f"{bcolors.BOLD}Paper authors{bcolors.ENDC}: F. Reghenzani, Z. Guo, L. Santinelli, W. Fornaciari")
print(f"{bcolors.BOLD}Corresponding Author{bcolors.ENDC}: Federico Reghenzani <federico.reghenzani@polimi.it>\n")

print(f"{bcolors.OKCYAN}Ok, as a first step I need to compile the C++ files under cpp_opt/ directory.")
print(f"The generated executable is required later for the tree exploration.{bcolors.ENDC}")

while True:
	print(f"{bcolors.WARNING}Continue? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		sys.exit(0)
	elif choice in yes:
		break

print("I'm executing `make` inside `cpp_opt` directory...")
os.chdir("cpp_opt")
ret = subprocess.call("make")
os.chdir("..")

if ret == 0:
	print(f"{bcolors.OKGREEN}Compiled successfully!{bcolors.ENDC}\n")
else:
	print(f"{bcolors.FAIL}Error occurred, exiting now. Please contact authors.{bcolors.ENDC}")
	sys.exit(1)

print(f"{bcolors.OKCYAN}Perfect, now I have some questions for you:{bcolors.ENDC}")
print(f" - How many threads should I use in simulation?")
print(f"   (ideally, this value should match the number of cores of your VM)")
print(f"   {bcolors.WARNING}Number of threads:{bcolors.ENDC} ", end='')
NR_THREADS = int(input())

print(f" - Which random seed you want to use? (integer)")
print(f"   (the data obtained in the paper used 12345 as random seed; however, the generation of")
print(f"   random numbers also depend on the number of threads previously selected, so small")
print(f"   differences are possible with respect to the data of the paper)")
print(f"   {bcolors.WARNING}Seed:{bcolors.ENDC} ", end='')
SEED = int(input())

print(f" - How many random task set you want to generate for each configuration? (integer)")
print(f"   Paper data has been generated with 1000 task sets for each scenario in EDF and EDF-VD case")
print(f"   while the full tree simulation uses 100 task sets.")
print(f"   {bcolors.WARNING}Number of tasksets:{bcolors.ENDC} ", end='')
NR_TASKSETS = int(input())
NR_TASKSETS_TREE = NR_TASKSETS

print(f"\n{bcolors.OKCYAN}Awesome, I will recap now my internal configuration which match the paper data")
print(f"(you can edit it by changing the variables inside this file):{bcolors.ENDC}")
print(f"  Number of task per task set to explore: ", NR_TASKS)
print(f"  Fault probablities per task set to explore: ", FAULT_P)
print(f"  Utilization range to explore: " + str(MIN_UTIL) + "% to " + str(MAX_UTIL) + "%")
print(f"\n{bcolors.OKCYAN}According to this configuration I will:{bcolors.ENDC}")
print(f"  Explore " + str(len(FAULT_P)*len(NR_TASKS)*(MAX_UTIL-MIN_UTIL)) + " scenarios")
print(f"  Generate a total of " + str(NR_TASKSETS*len(FAULT_P)*len(NR_TASKS)*(MAX_UTIL-MIN_UTIL)) + " random tasksets")

print(f"\n{bcolors.OKCYAN}Well, it's time for me to start working...{bcolors.ENDC}")
print(f"You can run three simulations: EDF, EDF-VD, and TREE (please check the paper)\n")

HAS_EDF_RUN=False

while True:
	print(f"{bcolors.WARNING}Do you want to run the EDF simulation? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		HAS_EDF_RUN = True
		experiments.run_sim(1, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
		print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
		break

HAS_EDF_VD_RUN = False

while True:
	print(f"{bcolors.WARNING}Do you want to run the EDF-VD simulation? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		HAS_EDF_VD_RUN = True
		experiments.run_sim(2, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
		print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
		break

NR_TASKSETS_TREE = 100
TREE_TASKSET_PREFIX = "precomputed/"

while True:
	print(f"{bcolors.WARNING}Do you want to run the TREE simulation?\n{bcolors.ENDC}", end='')
	print(f"WARNING: Depending on your machine this may take {bcolors.FAIL}HOURS{bcolors.ENDC} or {bcolors.FAIL}DAYS{bcolors.ENDC}.\n", end='')
	print(f" As a rough comparison, our exploration ran on a dedicated 24-core high\n", end='')
	print(f" performance machine for a few days.\n", end='')
	print(f"{bcolors.OKCYAN} If you skip this, you can still use pre-computed results (just say N){bcolors.ENDC}\n", end='')
	print(f"{bcolors.WARNING}(Y/N) ", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		print(f"{bcolors.FAIL}Are you really sure? On a standard PC it may take DAYS. (Y/N) {bcolors.ENDC}", end='')
		choice = input().lower()
		if choice in yes:
			NR_TASKSETS_TREE = NR_TASKSETS
			TREE_TASKSET_PREFIX = ""
			experiments.run_sim(3, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
			print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
			break


print(f"{bcolors.OKCYAN}\n\nAlright. I will compute the schedulable+compliance ratio (ref. last\n{bcolors.ENDC}", end='')
print(f"{bcolors.OKCYAN}column Table III)\n\n{bcolors.ENDC}", end='')


if HAS_EDF_RUN:

	process = Popen(["./helpers/get_percentage.sh", "results/results_edf_1e-03.txt", str(NR_TASKSETS)], stdout=PIPE)
	(output, err) = process.communicate()
	exit_code = process.wait()

	print(f"- EDF: " + output.decode("utf-8") + "%")

if HAS_EDF_VD_RUN:

	process = Popen(["./helpers/get_percentage.sh", "results/results_edf_vd_1e-05.txt", str(NR_TASKSETS)], stdout=PIPE)
	(output, err) = process.communicate()
	exit_code = process.wait()

	print(f"- EDF-VD (MC) @ 10^-5: " + output.decode("utf-8") + "%")

	process = Popen(["./helpers/get_percentage.sh", "results/results_edf_vd_1e-04.txt", str(NR_TASKSETS)], stdout=PIPE)
	(output, err) = process.communicate()
	exit_code = process.wait()

	print(f"- EDF-VD (MC) @ 10^-4: " + output.decode("utf-8") + "%")

	process = Popen(["./helpers/get_percentage.sh", "results/results_edf_vd_1e-03.txt", str(NR_TASKSETS)], stdout=PIPE)
	(output, err) = process.communicate()
	exit_code = process.wait()

	print(f"- EDF-VD (MC) @ 10^-3: " + output.decode("utf-8") + "%")



process = Popen(["./helpers/get_percentage.sh", "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-05.txt", str(NR_TASKSETS_TREE)], stdout=PIPE)
(output, err) = process.communicate()
exit_code = process.wait()

print(f"- TREE @ 10^-5: " + output.decode("utf-8") + "%")

process = Popen(["./helpers/get_percentage.sh", "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-04.txt", str(NR_TASKSETS_TREE)], stdout=PIPE)
(output, err) = process.communicate()
exit_code = process.wait()

print(f"- TREE @ 10^-4: " + output.decode("utf-8") + "%")

process = Popen(["./helpers/get_percentage.sh", "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-03.txt", str(NR_TASKSETS_TREE)], stdout=PIPE)
(output, err) = process.communicate()
exit_code = process.wait()

print(f"- TREE @ 10^-3: " + output.decode("utf-8") + "%")

print(f"{bcolors.OKCYAN}\nNow it's time to plot something...\n{bcolors.ENDC}", end='')

choice = 1
while True:
	print(f"{bcolors.OKCYAN}\nWhich plot do you like to generate?\n{bcolors.ENDC}", end='')
	print(f" 1 - EDF\n{bcolors.ENDC}", end='')
	print(f" 2 - EDF-VD (MC) @ 10^-5\n{bcolors.ENDC}", end='')
	print(f" 3 - EDF-VD (MC) @ 10^-4\n{bcolors.ENDC}", end='')
	print(f" 4 - EDF-VD (MC) @ 10^-3\n{bcolors.ENDC}", end='')
	print(f" 5 - TREE @ 10^-5:\n{bcolors.ENDC}", end='')
	print(f" 6 - TREE @ 10^-4:\n{bcolors.ENDC}", end='')
	print(f" 7 - TREE @ 10^-3:\n{bcolors.ENDC}", end='')
	print(f" 0 - EXIT\n{bcolors.ENDC}", end='')
	print(f"{bcolors.WARNING} (0-7): {bcolors.ENDC}", end='')
	choice = int(input())
	
	
	filename = ""
	nr_tasksets = NR_TASKSETS
	if choice == 0:
		break;
	elif choice == 1:
		filename = "results/results_edf_1e-03.txt"
	elif choice == 2:
		filename = "results/results_edf_vd_1e-05.txt"
	elif choice == 3:
		filename = "results/results_edf_vd_1e-04.txt"
	elif choice == 4:
		filename = "results/results_edf_vd_1e-03.txt"
	elif choice == 5:
		filename = "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-05.txt"
		nr_tasksets = NR_TASKSETS_TREE
	elif choice == 6:
		filename = "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-04.txt"
		nr_tasksets = NR_TASKSETS_TREE
	elif choice == 7:
		filename = "results/" + TREE_TASKSET_PREFIX + "results_tree_1e-03.txt"
		nr_tasksets = NR_TASKSETS_TREE

	if filename != "":
		process = Popen(["python3", "./helpers/plot_single.py", filename, str(nr_tasksets)], stdout=PIPE)
		(output, err) = process.communicate()
		exit_code = process.wait()
	
