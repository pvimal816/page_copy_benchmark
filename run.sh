for thread_cnt in `seq 0 6`; do
    for data_size in `seq 0 16`; do
        sum=0
        for i in `seq 0 5`; do
            sum=$(echo $sum + $(numactl -N 0 -m 0 ./internode $((1<<data_size)) $((1<<thread_cnt))) | bc | awk '{printf "%f", $0}')
        done
        printf "$sum, " >> stats_dram_to_dram.csv;
    done
    printf "\n" >> stats_dram_to_dram.csv;
done