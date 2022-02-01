"""
This Python file contains the functions to compute the schedulability
condition for the EDF-VD algorithm with multiple levels.

Based on the algorithm presented in the following paper:
Sanjoy Baruah, Vincenzo Bonifaci, Gianlorenzo D'angelo, Haohan Li, Alberto Marchetti-Spaccamela, Suzanne Van Der Ster, and Leen Stougie. 2015. Preemptive Uniprocessor Scheduling of Mixed-Criticality Sporadic Task Systems. J. ACM 62, 2, Article 14 (May 2015), 33 pages. DOI:https://doi.org/10.1145/2699435


Equation references below refers to the paper above.

"""

def util(lower, upper, n_tasks, crit_level, U):	# Utilization computation, ref. just above [Eq. 1] (unnamed equation)
    assert(upper <= lower)

    value_sum = 0
    for i in range(0,n_tasks):
        if lower == crit_level[i]:
            value_sum += U[i][upper-1]
    return value_sum

def basic_check(n_tasks, crit_level, U, K):	# Trivial schedulability according to [Eq. 2]
    curr_sum = 0
    for l in range(1, K+1):
        curr_sum += util(l,l, n_tasks, crit_level, U)
    if curr_sum <= 1:
        return True
    else:
        return False

    
def compare_term(k, K, n_tasks, crit_level, U):	# [Eq.3]
    numerator = 0
    for l in range(k+1, K+1):
        numerator += util(l,l, n_tasks, crit_level, U)

    denominator = 0
    for l in range(1, k+1):
        denominator += util(l,l, n_tasks, crit_level, U)
    
    if denominator == 0:
        denominator = 10e-9	# Just to avoid infinite results...
    
    return (1-numerator) / denominator

def should_smaller_term(k, K, n_tasks, crit_level, U): # [Eq.3]
    numerator = 0
    for l in range(k+1, K+1):
        numerator += util(l,k, n_tasks, crit_level, U)

    denominator = 0
    for l in range(1, k+1):
        denominator += util(l,l, n_tasks, crit_level, U)

    if denominator == 1:
        denominator = 1-10e-9	# Just to avoid infinite results...
    
    return numerator / ( 1 - denominator)

def condition_1(k_small, n_tasks, crit_level, U):	# Left condition with respect to the "and" [Eq. 3]
    curr_sum = 0
    for l in range(1, k_small+1):
        curr_sum += util(l,l, n_tasks, crit_level, U)
    if (1 - curr_sum) > 0:
        return True
    else:
        return False

def condition_2(k_small, K, n_tasks, task_crit_levels, U):  # Right condition with respect to the "and" [Eq. 3]
    return should_smaller_term(k_small, K, n_tasks, task_crit_levels, U) <= compare_term(k_small, K, n_tasks, task_crit_levels, U)        

