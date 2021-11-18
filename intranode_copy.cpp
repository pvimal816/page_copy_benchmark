#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PAGE_4KB 4096

int main(int argc, char *argv[]){
    if(argc!=2){
        printf("Usage: ./main page_count\n");
        exit(0);
    }

    int page_count = atoi(argv[1]);

    fprintf(stderr, "Copying %d 4KB pages.\n", page_count);

    char* src = (char *) malloc(PAGE_4KB * page_count);
    char* dst = (char *) malloc(PAGE_4KB * page_count);

	memset(src, 1, PAGE_4KB * page_count);
	memset(dst, 1, PAGE_4KB * page_count);
	
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
    memcpy(dst, src, page_count*PAGE_4KB);

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

    printf("Total_copy_cycles\tBegin_timestamp\tEnd_timestamp\n"
		   "%lu\t%lu\t%lu\n",
		   (end-begin), begin, end);

    printf("Bandwidth(GB/Cycles)=%f\n", (PAGE_4KB*(long long)page_count)/(float)(end-begin));

    return 0;
}