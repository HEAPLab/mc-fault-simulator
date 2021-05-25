import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np
import sys, os

def myplot(filename):
    nr_tasks,util,sched = np.loadtxt(filename,delimiter=' ',usecols=(0,1,2),unpack=True)

    data_x = {}
    data_y = {}
    
    for i in range(0,len(nr_tasks)):

        n_t = int(nr_tasks[i])

        if not (n_t in data_x):
            data_x[n_t] = []
            data_y[n_t] = []

        data_x[n_t].append(util[i])
        data_y[n_t].append(sched[i] / 1000)


    fig, ax = plt.subplots( nrows=1, ncols=1, figsize=(2*1.61,2*1) )
    ax.plot(data_x[5], data_y[5]        , color=[0, 0.5, 1]);
    ax.plot(data_x[10], data_y[10], '--', color=[0, 0, 1]);
    ax.plot(data_x[25], data_y[25], '-.', color=[0, 0.5, 0.5]);
    ax.plot(data_x[50], data_y[50], ':',  color=[0, 0, 0.25]);
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.set_xlabel("Utilization")
    ax.legend(['n=5','n=10','n=25','n=50'])
    fig.savefig(os.path.splitext(sys.argv[1])[0]+".pdf", bbox_inches='tight')
    plt.close(fig)
    
myplot(sys.argv[1])
