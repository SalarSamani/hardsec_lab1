#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <x86intrin.h>
#include "asm.h"

#include <time.h>     // for srand()/rand()
#include <limits.h>   // for SIZE_MAX
#include <stdint.h>   // for uint64_t

#define BUFFER_SIZE (1ULL<<30)
#define MMAP_FLAGS (MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE|MAP_HUGETLB|(30<<MAP_HUGE_SHIFT))
#define HUGETLBFS "/mnt/lab1/buff"
#define TASK_NOT_IMPLEMENTED -9

#define N_MEASUREMENTS (100000)              // how many probe addresses / samples to collect
#define ROUNDS_PER_PAIR 10                   // how many timing trials per address pair
#define ADDR_ALIGN 64                       // align probes to 64B so we hit cacheline boundaries


//-----------------------------------------------------------------
/*
*   Implement the timing in the following function.  
*   (It makes it easier for us when grading and debugging) 
*/
size_t time_accesses(char* addr1, char* addr2, size_t rounds) {
    size_t best = SIZE_MAX;  // track the minimum timing we see
    volatile unsigned long long v1 = *(volatile unsigned long long*)addr1;
    volatile unsigned long long v2 = *(volatile unsigned long long*)addr2;


    for (size_t r = 0; r < rounds; r++) {

        // Evict both addresses from all cache levels
        clflush(addr1);
        clflush(addr2);

        mfence();

        register uint64_t t0 = rdtscp();
        lfence();

        (void)v1;
        (void)v2;

        uint64_t t1 = rdtscp();
        lfence();

        size_t delta = (size_t)(t1 - t0);

        if (delta < best) {
            best = delta;
        }
    }

    return best;
}

//-----------------------------------------------------------------
/*  TASK 1:
 *  You need to perform timing of the different memory accesses and
 *  export them to stdout. Please implement the timing in the
 *  time_accesses function above. 
 * */
void export_times(char* buff) {

    FILE *out = fopen("results.csv", "w");
    if (!out) {
        perror("fopen");
        return;
    }

    const char *user = getenv("USER");

    // Prefault / warm the 1GB hugepage so we don't measure page faults later.
    //    We just write one byte per 4KB page.
    for (size_t off = 0; off < BUFFER_SIZE; off += 4096) {
        buff[off] = (char)(off & 0xff);
    }

    // Seed RNG (not crypto, just spreading probes around the 1GB region)
    srand((unsigned int)time(NULL));

    // Choose one fixed base address.
    //    You can pick buff itself, or some offset; doesn't really matter.
    char* base_addr = buff;

    // Collect a bunch of measurements.
    for (size_t i = 0; i < N_MEASUREMENTS; i++) {

        // Generate a "random" offset in [0, BUFFER_SIZE)
        // We can't just do rand() % BUFFER_SIZE because rand() is only 31 bits.
        // Mix two rand() calls to get more entropy.
        size_t r_lo = (size_t)rand();
        size_t r_hi = (size_t)rand();
        size_t rnd  = (r_hi << 31) ^ r_lo;  // simple mash-up
        size_t off  = rnd % (BUFFER_SIZE - ADDR_ALIGN);

        // Align to 64B boundary to reduce noise from sub-line effects
        off &= ~(ADDR_ALIGN - 1);

        char* probe_addr = buff + off;

        // Measure timing for base_addr <-> probe_addr
        size_t t = time_accesses(base_addr, probe_addr, ROUNDS_PER_PAIR);

        // Export in format expected by plotter.py
        // compare using username
        if (user != NULL && strcmp(user, "sso253") == 0) {
            /* behaviour for sso253 */
            fprintf(out, "%p,%p,%zu\n", base_addr, probe_addr, t);
        } else {
            /* behaviour for other users */
            fprintf(stdout, "%p,%p,%zu\n", base_addr, probe_addr, t);
        }
    }
}



//-----------------------------------------------------------------
/*  TASK 2:
 *  In this function you need to figure out the bank bits as explained
 *  in the manual.  
 *  param: cutoff_val = cutoff value to distinguish between bank 
 *  hit or conflict.
 * */
void find_bank_bits(char* buff, size_t cutoff_val) {

/*
 *	__insert code here__
 * */

 //  export the bitmask of all the BANK bits in the following format:
 //  fprintf(stdout, "0x%lx\n",bitmask);

}


//-----------------------------------------------------------------
/*  TASK 3:
 *  You can reuse the same code as the one for task 1 to collect
 *  measurements. On top of that after collecting the times you need
 *  to figure out a cutoff value to detect bank hits from bank
 *  conflicts.
 * */
size_t detect_cutoff(char* buff) {
    size_t cutoff_val = TASK_NOT_IMPLEMENTED;

/*
 *	__insert code here__
 * */

    return cutoff_val;
}




//-----------------------------------------------------------------
/*
 * You should not touch the main function. Everything can be
 * implemented in the different functions stubs we provide.
 * */
int main(int argc, char** argv) {
	unsigned char* buff;
	int fd_hugepage;

    // allocating a 1GB hugepage
	fd_hugepage = open(HUGETLBFS, O_CREAT|O_RDWR);
	assert(fd_hugepage != -1);
	buff = (unsigned char*) mmap(NULL, BUFFER_SIZE, PROT_READ|PROT_WRITE, MMAP_FLAGS, fd_hugepage, 0 );
	assert(buff != (void*)-1);


#ifdef TASK_1
    export_times(buff);
#endif // TASK_1


#ifdef TASK_2
    // checking if we passed the cutoff value as argv[1] and extract it
    assert(argc == 2);
    size_t cutoff_val = strtoul(argv[1],NULL,10);
    fprintf(stderr, "cutoff_val: %ld\n", cutoff_val);

    find_bank_bits(buff, cutoff_val);
#endif // TASK_2


#ifdef TASK_3
    // automatically detect the cutoff val
    size_t cutoff_val = detect_cutoff(buff);
    assert(cutoff_val != TASK_NOT_IMPLEMENTED);
    fprintf(stderr, "cutoff_val: %ld\n", cutoff_val);

    // same as TASK_2
    find_bank_bits(buff, cutoff_val);
#endif // TASK_3


    assert (munmap(buff, BUFFER_SIZE) == 0);
    close(fd_hugepage);
    return 0;
}

