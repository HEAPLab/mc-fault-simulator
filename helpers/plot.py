import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import sys


def plot(filename):

    x,y,z =np.loadtxt(filename,delimiter=' ',usecols=(0,1,2),unpack=True)

    len_el_x = len(set(x))
    len_el_y = len(set(y))

#    print(set(x))
#    print(set(y))

    plt_z = np.zeros((len_el_x, len_el_y))

    for i in range(len(z)):
	    i_row = int(i%len_el_y)
	    i_col = int(i/len_el_y)
	    plt_z[i_col][i_row] = z[i]

    fig, ax = plt.subplots()
    fig.set_size_inches(4, 2)
    plt.xlabel(r'U (utilization)')
    plt.ylabel(r'$n$ (nr. tasks)')

    ticks  = [0, 20, 40, 60, 80, 100] 
    labels  = [0, 0.2, 0.4, 0.6, 0.8, 1] 

    ax.set_xticks(ticks)
    ax.set_xticklabels(labels)

    im=plt.imshow(plt_z, cmap='bone', aspect='auto', vmin=0, vmax=1000)
    plt.gca().invert_yaxis()
    plt.ylim(0,50)
    plt.xlim(0,100)

    cax = fig.add_axes([ax.get_position().x1+0.01,ax.get_position().y0,0.02,ax.get_position().height])
    plt.colorbar(im, cax=cax) # Similar to fig.colorbar(im, cax = cax)


    #plt.colorbar(im,fraction=0.023, pad=0.05)

plot(sys.argv[1])
plt.savefig(sys.argv[1]+".pdf")

