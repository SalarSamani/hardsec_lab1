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
    size_t delta_t = 0;
/*
 *	__insert code here__
 * */
    return delta_t;
}

//-----------------------------------------------------------------
/*  TASK 1:
 *  You need to perform timing of the different memory accesses and
 *  export them to stdout. Please implement the timing in the
 *  time_accesses function above. 
 * */
void export_times(char* buff) {

/*
 *	__insert code here__
 * */

 //  export the output in the following format: 
 //  fprintf(stdout, "%p,%p,%ld\n",base_addr, probe_addr, time);

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

