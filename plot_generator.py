import numpy as np; np.random.seed(0)
import seaborn as sns; sns.set_theme()
import pandas as pd
import matplotlib.pyplot as plt

def main():
    # filename = "stats_dram_to_dram.csv"
    # filename = "stats_pmem_to_pmem.csv"
    # filename = "stats_pmem_to_dram.csv"
    filename = "stats_dram_to_pmem.csv"

    sourceNodeType = filename.split("_")[1]
    destNodeType = filename.split("_")[3].split(".")[0]

    df = pd.read_csv(filename, names=["order_{0}".format(i) for i in range(0, 17)], index_col=False)
    df.sort_index(axis='index', ascending=False, inplace=True)
    print(df.values)
    sns.set(font_scale=1.2)
    sns.set(rc={'figure.figsize':(11.7,8.27)})
    ax = sns.heatmap(df, annot=True, fmt="0.1f", cbar_kws={"shrink": 1.5}, cbar=False)
    ax.set_xticklabels([round((1<<i)/(1<<8), 2) for i in range(0, df.shape[1])], fontsize=12)
    print((ax.get_ymajorticklabels()))
    
    ax.set_yticklabels([1<<i for i in range(df.shape[0]-1, -1, -1)], fontsize=12)
    ax.set(xlabel='datasize(MBs)', ylabel='# of threads')
    ax.set(title='Relation between copy throughput(Bytes/Cycles), threads and datasize for {0} to {1} transfer'.format(sourceNodeType, destNodeType))
    plt.savefig("{0}_to_{1}_data_copy_stats.png".format(sourceNodeType, destNodeType))
main()