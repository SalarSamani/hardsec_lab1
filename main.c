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

#define BUFFER_SIZE (1ULL<<30)
#define MMAP_FLAGS (MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE|MAP_HUGETLB|(30<<MAP_HUGE_SHIFT))
#define HUGETLBFS "/mnt/lab1/buff"
#define TASK_NOT_IMPLEMENTED -9


//-----------------------------------------------------------------
/*
*   Implement the timing in the following function.  
*   (It makes it easier for us when grading and debugging) 
*/
size_t time_accesses(char* addr1, char* addr2, size_t rounds) {
    size_t delta = 0;
    size_t min_delta = ~0;
    for (size_t i = 0; i < rounds; i++) {
        clflush(addr1);
        clflush(addr2);     
        lfence();
        cpuid();
        uint64_t t1 = rdtscp();
        *(volatile char*)addr1;
        *(volatile char*)addr2;
        uint64_t t2 = rdtscp();
        delta = t2 - t1;
        if (delta < min_delta) {
            min_delta = delta;
        }
    }
    return min_delta;
}


#define CACHELINE       64
#define ROUNDS          100
#define TOTAL_PROBES    20000

void export_times(char* buff) {
    for (size_t i = 0; i < TOTAL_PROBES; i++) {
        size_t offset = ((size_t)rand() % (BUFFER_SIZE / CACHELINE)) * CACHELINE;
        char* pair = buff + offset;
        size_t time = time_accesses(buff, pair, ROUNDS);
        fprintf(stdout, "%p,%p,%zu\n", buff, pair, time);
    }
}

//-----------------------------------------------------------------
/*  TASK 2:
 *  In this function you need to figure out the bank bits as explained
 *  in the manual.  
 *  param: cutoff_val = cutoff value to distinguish between bank 
 *  hit or conflict.
 * */
 //-----------------------------------------------------------------
#define CANDIDATES 4096

static inline char* random_line(char* base){
    size_t off = ((size_t)rand() % (BUFFER_SIZE / CACHELINE)) * CACHELINE;
    return base + off;
}

static char* find_conflict_addr(char* base, size_t limit){
    for(;;){
        char* best_a = NULL;
        size_t best_t = 0;
        for(size_t i=0;i<CANDIDATES;i++){
            char* a = random_line(base);
            size_t t = time_accesses(base, a, ROUNDS);
            if(t > best_t){ best_t = t; best_a = a; }
        }
        if(best_t > limit) return best_a;
    }
}

void find_bank_bits(char* buf, size_t limit){
    char* base = buf;
    char* conflict = find_conflict_addr(base, limit);
    unsigned long mask = 0;
    for(size_t b=6;b<30;b++){
        char* test = (char*)((uintptr_t)conflict ^ (1ULL<<b));
        if(time_accesses(base, test, ROUNDS) < limit) mask |= (1ULL<<b);
    }
    printf("0x%lx\n", mask);
}


//-----------------------------------------------------------------
/*  TASK 3:
*  You can reuse the same code as the one for task 1 to collect
 *  measurements. On top of that after collecting the times you need
 *  to figure out a cutoff value to detect bank hits from bank
 *  conflicts.
 * */
#define SAMPLES 20000
#define NUM_BINS 512

size_t detect_cutoff(char* buf){
    size_t times[SAMPLES];
    for(size_t i=0;i<SAMPLES;i++){
        size_t off = ((size_t)rand() % (BUFFER_SIZE/CACHELINE)) * CACHELINE;
        times[i] = time_accesses(buf, buf+off, ROUNDS);
    }

    size_t min_t = times[0], max_t = times[0];
    for(size_t i=1;i<SAMPLES;i++){
        if(times[i] < min_t) min_t = times[i];
        if(times[i] > max_t) max_t = times[i];
    }
    if(min_t == max_t) return min_t;

    double bin_width = (double)(max_t - min_t) / NUM_BINS;
    size_t hist[NUM_BINS] = {0};
    for(size_t i=0;i<SAMPLES;i++){
        size_t b = (size_t)((times[i]-min_t)/bin_width);
        if(b >= NUM_BINS) b = NUM_BINS-1;
        hist[b]++;
    }

    double total_w = 0, total_sum = 0;
    for(size_t b=0;b<NUM_BINS;b++){
        double center = min_t + (b+0.5)*bin_width;
        total_w += hist[b];
        total_sum += center*hist[b];
    }

    double left_w = 0, left_sum = 0, best_score = -1, best_edge = min_t;
    for(size_t b=0;b<NUM_BINS-1;b++){
        double center = min_t + (b+0.5)*bin_width;
        left_w += hist[b];
        left_sum += center*hist[b];
        double right_w = total_w - left_w;
        if(left_w == 0 || right_w == 0) continue;
        double mean_left = left_sum/left_w;
        double mean_right = (total_sum-left_sum)/right_w;
        double score = left_w*right_w*(mean_left-mean_right)*(mean_left-mean_right);
        if(score > best_score){
            best_score = score;
            best_edge = min_t + (b+1)*bin_width;
        }
    }
    return (size_t)best_edge;
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

