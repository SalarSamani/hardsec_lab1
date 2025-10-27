#include <inttypes.h>

static inline __attribute__((always_inline)) void clflush(void* p) {
	asm volatile("clflush (%0)\n"::"r"(p));
}

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
	uint64_t lo, hi;
	asm volatile("rdtsc\n" : "=a" (lo), "=d" (hi) :: "rcx");
	return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;
	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");
	return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) void lfence() {
	asm volatile ("lfence\n");
}

static inline __attribute__((always_inline)) void sfence() {
	asm volatile ("sfence\n");
}

static inline __attribute__((always_inline)) void mfence() {
	asm volatile ("mfence\n");
}

static inline __attribute__((always_inline)) void cpuid(void) {
	asm volatile ("cpuid\n\t" ::: "%rax", "%rbx", "%rcx", "%rdx");
}
