import sys, os, subprocess
from main_scripts import experiments

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

print(f"{bcolors.OKCYAN}Ok, as a first step I need to compile the C++ files under cpp/ directory.")
print(f"The generated executable is required later for the tree exploration.{bcolors.ENDC}")

while True:
	print(f"{bcolors.WARNING}Continue? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		sys.exit(0)
	elif choice in yes:
		break

print("I'm executing `make` inside `cpp` directory...")
os.chdir("cpp")
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
print(f"   (to obtain the exactly same data of the paper, enter 12345 here)")
print(f"   {bcolors.WARNING}Seed:{bcolors.ENDC} ", end='')
SEED = int(input())

print(f" - How many random task set you want to generate for each configuration? (integer)")
print(f"   WARNING: it significantly affects the simulation time. Paper has been generated")
print(f"   with 1000 task sets for each scenario, but I suggest you to start with 50-100,")
print(f"   otherwise the full TREE simulation may take many hours to complete. If you plan")
print(f"   to do not run the TREE simulation, you can also put 1000.")
print(f"   {bcolors.WARNING}Number of tasksets:{bcolors.ENDC} ", end='')
NR_TASKSETS = int(input())

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

while True:
	print(f"{bcolors.WARNING}Do you want to run the EDF simulation? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		experiments.run_sim(1, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
		print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
		break

while True:
	print(f"{bcolors.WARNING}Do you want to run the EDF-VD simulation? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		experiments.run_sim(2, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
		print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
		break

while True:
	print(f"{bcolors.WARNING}Do you want to run the TREE simulation? (Y/N) {bcolors.ENDC}", end='')
	choice = input().lower()
	if choice in no:
		break
	elif choice in yes:
		print(f"\n{bcolors.OKCYAN}The TREE may take long time to complete (each taskset may require from a few")
		print(f"milliseconds to several minutes). Also, the following progress bars are non-linear")
		print(f"and usually slower in the range 20-40%.{bcolors.ENDC}")
		experiments.run_sim(3, SEED, NR_TASKS, MIN_UTIL, MAX_UTIL, NR_TASKSETS, NR_THREADS, FAULT_P)
		print(f"{bcolors.OKGREEN}Success!{bcolors.ENDC}\n")
		break


