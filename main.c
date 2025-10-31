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
#include <time.h>
void find_bank_bits(char* buff, size_t cutoff_val) {
    size_t offset;
    char *conflict, *test;
    size_t t;
    size_t mask = 0;
    
    srand(time(NULL));
    while (1) {
        offset = ((size_t)(rand()) % (BUFFER_SIZE));
        conflict = buff + offset;
        t = time_accesses(buff, test, ROUNDS);
        printf("*****\n");
        if (t > cutoff_val) break;        
    }
    for (size_t i = 6; i < 30; i++) {
        size_t test_bit = (1ULL << i);
        char* test = (char*)((size_t)conflict ^ test_bit);
        size_t t = time_accesses(buff, test, ROUNDS);
        if (t < cutoff_val) mask |= (1ULL << i);
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
size_t detect_cutoff(char* buff) {
    size_t cutoff_val = TASK_NOT_IMPLEMENTED;

/*
 *	__insert code here__
 * */
    struct Data
    {
        char* base_addr;
        char* probe_addr;
        size_t time;
    };

    char* base_addr = buff; // Always the same base
    size_t rounds = 200; // TODO: How many rounds?
    size_t total_probes = 20000; // TODO: How many probes?
    struct Data dataset[total_probes];

    for (size_t i = 0; i < total_probes; i++) {
        size_t offset = (rand() % (BUFFER_SIZE / 64)) * 64; // 64B cacheline // TODO: DRAMA uses 128?
        char* probe_addr = base_addr + offset;

        size_t time = time_accesses(base_addr, probe_addr, rounds);

        struct Data data = {base_addr, probe_addr, time};
        dataset[i] = data;
    }

    // K-means alg to find 2 clusters
    double centroids[2];
    int* labels = calloc(total_probes, sizeof(int));

    // Select min and max for initial centroids 
    centroids[0] = dataset[0].time;
    centroids[1] = dataset[0].time;
    for (int i = 1; i < total_probes; i++) {
        if (dataset[i].time < centroids[0]) centroids[0] = dataset[i].time;
        if (dataset[i].time > centroids[1]) centroids[1] = dataset[i].time;
    }

    for (int i = 0; i < 100; i++) {
        int label_changed = 0;

        // Assign points to the closest centroid
        for (int j = 0; j < total_probes; j++) {
            double dist0 = dataset[j].time - centroids[0];
            if (dist0 < 0) dist0 = -dist0;
            double dist1 = dataset[j].time - centroids[1];
            if (dist1 < 0) dist1 = -dist1;
            int new_label = (dist0 < dist1) ? 0 : 1;

            if (labels[j] != new_label) {
                labels[j] = new_label;
                label_changed = 1;
            }
        }

        // If no label changed, it is done
        if (!label_changed) break;

        // Calculate centroids
        size_t sum[2] = {0, 0};
        int count[2] = {0, 0};
        for (int j = 0; j < total_probes; j++) {
            sum[labels[j]] += dataset[j].time;
            count[labels[j]]++;
        }

        for (int j = 0; j < 2; j++) {
            if (count[j] > 0)
                centroids[j] = sum[j] / count[j];
        }
    }

    cutoff_val = (centroids[0] + centroids[1]) / 2; // Middle point of the cluster centers

    free(labels);
    
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

