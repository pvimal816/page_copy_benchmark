/**
 * This program measures the data copy time from NUMA node 0 to 1.
 * For that on each of NUMA node 0 and 1 a single page is allocated and
 * page on node 0 is copied to page on node 1.
 * The time taken in page allocation is not counted.
 * 
 * run this program by binding it to node 0 using numactl.
 */

#include <stdio.h>
#include <stdlib.h>
#include <numa.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mempolicy.h>
#include <assert.h>

#include <vector>
#include <thread>

#define SOURCE_NODE 0
#define DEST_NODE 1
#define THREAD_COUNT 1

#define PAGE_4K (4UL*1024)
#define PAGE_2M (PAGE_4K*512)

void copy(volatile unsigned long* src, volatile unsigned long* dst, size_t byteCnt){
    for(int i=0; i<byteCnt; i+=sizeof(unsigned long), src++, dst++)
        *dst=*src;
}

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Usage: ./main page_count thread_count\n");
        exit(0);
    }

    int page_size = PAGE_4K;
    int page_count = atoi(argv[1]);
    int thread_count = atoi(argv[2]);

    int nr_nodes;
    struct bitmask *old_nodes;
    struct bitmask *new_nodes;

    nr_nodes = numa_max_node()+1;
    old_nodes = numa_bitmask_alloc(nr_nodes);
    new_nodes = numa_bitmask_alloc(nr_nodes);
    numa_bitmask_setbit(old_nodes, SOURCE_NODE);
    numa_bitmask_setbit(new_nodes, DEST_NODE);

    if (nr_nodes < 2) {
            printf("A minimum of 2 nodes is required for this test.\n");
            exit(1);
    }

    void **local_addr;
    int *local_status;
    int *local_nodes;
    void **remote_addr;
    int *remote_status;
    int *remote_nodes;

    local_addr = (void**)malloc(sizeof(char *) * page_count);
    local_status = (int*)malloc(sizeof(int *) * page_count);
    local_nodes = (int*)malloc(sizeof(int *) * page_count);

    remote_addr = (void**) malloc(sizeof(char *) * page_count);
    remote_status = (int*) malloc(sizeof(int *) * page_count);
    remote_nodes = (int*) malloc(sizeof(int *) * page_count);

    char* local_page = (char*)aligned_alloc(page_size, page_count*page_size);
    char* remote_page = (char*)aligned_alloc(page_size, page_count*page_size);
    
    if(page_size==PAGE_2M){
        madvise(local_page, page_count*page_size, MADV_HUGEPAGE);
        madvise(remote_page, page_count*page_size, MADV_HUGEPAGE);
    }

    memset(local_page, 0, page_count*page_size);
    memset(remote_page, 1, page_count*page_size);

    for (int i = 0; i < page_count; i++) {
        local_addr[i] = local_page + i * page_size;
        local_nodes[i] = SOURCE_NODE;
        local_status[i] = -123;
        remote_addr[i] = remote_page + i * page_size;
        remote_nodes[i] = DEST_NODE;
        remote_status[i] = -123;
    }

    // move local_page to node 0
    if(numa_move_pages(0, page_count, local_addr, local_nodes, local_status, MPOL_MF_MOVE)){
        perror("Error in moving page to local_node: ");
        exit(-1);
    }

    // move remote_page to node 1
    if(numa_move_pages(0, page_count, remote_addr, remote_nodes, remote_status, MPOL_MF_MOVE)){
        perror("Error in moving page to remote_node: ");
        exit(-1);
    }

    unsigned long begin, end;
    unsigned int cycles_high, cycles_low;
    unsigned int cycles_high1, cycles_low1;

    asm volatile
	( "CPUID\n\t"
	  "RDTSC\n\t"
	  "mov %%edx, %0\n\t"
	  "mov %%eax, %1\n\t"
	  :
	  "=r" (cycles_high), "=r" (cycles_low)
	  ::
	  "rax", "rbx", "rcx", "rdx"
	);
	begin = ((uint64_t)cycles_high <<32 | cycles_low);

    // perform copy here
    // memcpy(remote_page, local_page, page_count*page_size);
    volatile unsigned long int *src=(unsigned long *) local_page;
    volatile unsigned long int *dst = (unsigned long*) remote_page;
    // for(int i=0; i<page_count*page_size; i+=8, src++, dst++)
    //     *dst=*src;
    
    std::vector<std::thread> workers;
    int chunkSize = (page_count*page_size)/thread_count;
    assert(chunkSize*thread_count == page_count*page_size);
    // assuming that thread_count devides page_count*page_size
    for(int i=0; i<thread_count; i++, src+=(chunkSize>>3), dst+=(chunkSize>>3))
        workers.push_back(std::thread(copy, src, dst, chunkSize));

    for(int i=0; i<thread_count; i++)
        workers[i].join();

    asm volatile
	( "RDTSCP\n\t"
	  "mov %%edx, %0\n\t"
	  "mov %%eax, %1\n\t"
	  "CPUID\n\t"
	  :
	  "=r" (cycles_high1), "=r" (cycles_low1)
	  ::
	  "rax", "rbx", "rcx", "rdx"
	);

    end = ((uint64_t)cycles_high1 <<32 | cycles_low1);

    unsigned long total_cycles = (end-begin);

    // printf("Total_copy_cycles\tBegin_timestamp\tEnd_timestamp\n"
	// 	   "%lu\t%lu\t%lu\n",
	// 	   (end-begin), begin, end);

    // printf("Bandwidth is %f B/Cycles.\n", (page_count*page_size)/(float)(total_cycles));

    printf("%f\n", (page_count*page_size)/(float)(total_cycles));

    return 0;
}
