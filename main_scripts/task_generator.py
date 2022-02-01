"""
Tools for generating task sets.
From: https://github.com/MaximeCheramy/simso
"""

import numpy as np
import random
import math


def UUniFastDiscard(n, u, nsets):
    sets = []
    while len(sets) < nsets:
        # Classic UUniFast algorithm:
        utilizations = []
        sumU = u
        for i in range(1, n):
            nextSumU = sumU * np.random.random() ** (1.0 / (n - i))
            utilizations.append(sumU - nextSumU)
            sumU = nextSumU
        utilizations.append(sumU)

        # If no task utilization exceeds 1:
        if all(ut <= 1 for ut in utilizations):
            sets.append(utilizations)

    return sets


def gen_uunifastdiscard(nsets, u, n):
    """
    The UUniFast algorithm was proposed by Bini for generating task
    utilizations on uniprocessor architectures.

    The UUniFast-Discard algorithm extends it to multiprocessor by
    discarding task sets containing any utilization that exceeds 1.

    Args:
        - `n`: The number of tasks in a task set.
        - `u`: Total utilization of the task set.
        - `nsets`: Number of sets to generate.

    Returns `nsets` of `n` task utilizations.
    """
    return UUniFastDiscard(n, u, nsets)


def gen_tasksets(utilizations, periods):
    """
    Take a list of task utilization sets and a list of task period sets and
    return a list of couples (c, p) sets. The computation times are truncated
    at a precision of 10^-10 to avoid floating point precision errors.

    Args:
        - `utilization`: The list of task utilization sets. For example::

            [[0.3, 0.4, 0.8], [0.1, 0.9, 0.5]]
        - `periods`: The list of task period sets. For examples::

            [[100, 50, 1000], [200, 500, 10]]

    Returns:
        For the above example, it returns::

            [[(30.0, 100), (20.0, 50), (800.0, 1000)],
             [(20.0, 200), (450.0, 500), (5.0, 10)]]
    """
    def trunc(x, p):
        return int(x * 10 ** p) / float(10 ** p)

    return [[(trunc(ui * pi, 6), trunc(pi, 6)) for ui, pi in zip(us, ps)]
            for us, ps in zip(utilizations, periods)]
            
            
