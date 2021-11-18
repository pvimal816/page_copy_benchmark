import numpy as np; np.random.seed(0)
import seaborn as sns; sns.set_theme()
import pandas as pd
import matplotlib.pyplot as plt

def main():
    df = pd.read_csv("stats_dram_to_dram.csv", names=["order_{0}".format(i) for i in range(0, 17)], index_col=False)
    print(df.values)
    uniform_data = df.values
    ax = sns.heatmap(uniform_data)
    plt.savefig("pmem_to_pmem_data_copy_stats.png")

main()