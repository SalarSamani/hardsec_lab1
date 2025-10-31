# DRAM Bank Bit Discovery (Assignment 1/HardSec) – Report

**Name:** Salar Soltanisamani  
**VUnetID:** sso253

---

## Overview
We allocate a 1 GiB buffer (huge page) and measure access times between a fixed “base” address and many other addresses. From these timings I:
1) collect raw measurements (Task 1),
2) infer which virtual address bits behave like DRAM **bank bits** (Task 2),
3) automatically find a good **cutoff** time that separates “conflict” vs “no‑conflict” (Task 3), then reuse it for Task 2.

All timing uses fences (`cpuid/rdtscp/lfence/mfence`) and `clflush` to eliminate cache effects and make the measurements more stable.

---

## Task 1 – Timing and Data Collection (Report)

#### Timing Function (what it does)
For a pair `(addr1, addr2)`, the function:
1. Flushes both addresses from cache (`clflush`).
2. Serializes timing (`lfence`, `cpuid`, `rdtscp`).
3. Loads `addr1` then `addr2` as `volatile` to force actual memory access.
4. Computes `delta = t2 - t1` in cycles.
5. Repeats for several rounds and returns the **minimum** delta (to avoid OS noise and random delays).

This minimum is a clean estimate of the real latency of the two back‑to‑back loads from DRAM.

**Why this matters**
- If `addr1` and `addr2` map to the **same DRAM bank but different rows**, the second load is slower (row conflict).
- If they are in **different banks** or the **same open row**, it is faster.
- Collecting many pairs gives a distribution with “fast” and “slow” clusters.

**What I export**
- I fix `base = buff` and pick many probe addresses inside the 1 GiB region (64‑byte aligned).
- For each pair, I print a CSV line:  
  `base_ptr,probe_ptr,time_cycles`

These samples are later useful to find a cutoff between fast and slow timings (but Task 1 only collects the data).

---

## Task 2 – Find Bank Bits
**Goal:** discover which virtual address bits (bit 6..29) are related to DRAM bank selection.

**Core idea:** if two addresses are in the **same bank but different row**, activating the second row is slower (precharge + new activate). So if I have one “conflict” address, and I flip a bank bit, I should move out of that bank and the time becomes fast again.

**What I did:**  
1) Choose a **conflict seed** address inside the 1 GiB region.  
2) For each bit `b` from 6 to 29, create `trial = conflict ^ (1<<b)` and time `(base, trial)`.  
3) If the time is **below** the cutoff, I count bit `b` as a bank bit.  
4) Print the final mask over bits 6..29.

---

## Task 3 – Detect Cutoff Automatically (Bonus)
**Goal:** find a threshold (in cycles) that separates fast cases (same open row / different bank) from slow cases (same bank, different row).

**Algorithm (histogram split with maximal between‑class separation):**
1) Collect many timings like Task 1.  
2) Build a histogram over the timing range (fixed number of bins).  
3) For each possible split between two bins, compute a score based on the two group means and sizes (the larger the gap and the more balanced the groups, the higher the score).  
4) Pick the split with the **best score** and use its boundary as the **cutoff**.  
5) Run the same bit‑flipping procedure from Task 2 with this cutoff to print the bank‑bit mask.

**Why this works:** timings often look like two clusters (fast vs slow). The split that maximizes between‑class separation is a simple way to pick a stable threshold without manual tuning.

