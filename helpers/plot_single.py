import matplotlib
#matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np
import sys, os

def myplot(filename, total_nr):
    nr_tasks,util,sched = np.loadtxt(filename,delimiter=' ',usecols=(0,1,2),unpack=True)

    data_x = {}
    data_y = {}
    
    for i in range(0,len(nr_tasks)):

        n_t = int(nr_tasks[i])

        if not (n_t in data_x):
            data_x[n_t] = []
            data_y[n_t] = []

        data_x[n_t].append(util[i])
        data_y[n_t].append(sched[i] / int(total_nr))


    fig, ax = plt.subplots( nrows=1, ncols=1, figsize=(2*1.61,2*1) )
    
    line_type = [ '-', '--', '-.', ':' ]
    colors    = [ [0, 0.5, 1], [0, 0, 1], [0, 0.5, 0.5], [0, 0, 0.25] ]
    
    i = 0
    for key in data_x:
        ax.plot(data_x[key], data_y[key], line_type[i], color=colors[i]);
        i = (i + 1) % 4

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.set_xlabel("Utilization")
    ax.legend(['n=5','n=10','n=25','n=50'])
#    fig.savefig(os.path.splitext(sys.argv[1])[0]+".pdf", bbox_inches='tight')
#    plt.close(fig)
    plt.show()
    
myplot(sys.argv[1], sys.argv[2])
