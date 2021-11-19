# source_node=(0 0 7 7)
# dest_node=(1 7 9 0)
# memory_node=(0 0 2 2)
# cpu_node=()
# transfer_type="dram_to_dram"

# ========== dram to dram ==========

# for thread_cnt in `seq 0 6`; do
#     for data_size in `seq 0 16`; do
#         sum=0
#         for i in `seq 0 5`; do
#             sum=$(echo $sum + $(numactl -N 0 -m 0 ./internode $((1<<data_size)) $((1<<thread_cnt)) 0 2) | bc | awk '{printf "%f", $0}')
#             echo $sum
#         done
#         printf "$sum, " >> stats_dram_to_dram.csv;
#     done
#     printf "\n" >> stats_dram_to_dram.csv;
# done

# ====== pmem to pmem ========

# for thread_cnt in `seq 0 6`; do
#     for data_size in `seq 0 16`; do
#         sum=0
#         for i in `seq 0 5`; do
#             sum=$(echo $sum + $(numactl -N 2 -m 7 ./internode $((1<<data_size)) $((1<<thread_cnt)) 7 9) | bc | awk '{printf "%f", $0}')
#             echo $sum
#         done
#         printf "$sum, " >> stats_pmem_to_pmem.csv;
#     done
#     printf "\n" >> stats_pmem_to_pmem.csv;
# done

# ====== pmem to dram ========

# for thread_cnt in `seq 0 6`; do
#     for data_size in `seq 0 16`; do
#         sum=0
#         for i in `seq 0 5`; do
#             sum=$(echo $sum + $(numactl -N 2 -m 7 ./internode $((1<<data_size)) $((1<<thread_cnt)) 7 0) | bc | awk '{printf "%f", $0}')
#             echo $sum
#         done
#         printf "$sum, " >> stats_pmem_to_dram.csv;
#     done
#     printf "\n" >> stats_pmem_to_dram.csv;
# done

# ====== dram to pmem ========

for thread_cnt in `seq 0 6`; do
    for data_size in `seq 0 16`; do
        sum=0
        for i in `seq 0 5`; do
            sum=$(echo $sum + $(numactl -N 0 -m 0 ./internode $((1<<data_size)) $((1<<thread_cnt)) 0 7) | bc | awk '{printf "%f", $0}')
            echo $sum
        done
        printf "$sum, " >> stats_dram_to_pmem.csv;
    done
    printf "\n" >> stats_dram_to_pmem.csv;
done