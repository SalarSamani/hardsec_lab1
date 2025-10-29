import sys
import matplotlib.pyplot as plt

times = []
for l in sys.stdin:
    triplet = l.split(",")
    t = int(triplet[2])
    times.append(t)

if not times:
    print("Task 1 not implemented it seems...")
    exit(1)

# --- Added: count below/above 600 cycles ---
threshold = 1000
below = sum(1 for t in times if t <= threshold)
above = len(times) - below
print(f"<= {threshold}: {below}, > {threshold}: {above}")
print(f"Percentage <= {threshold}: {below / len(times) * 100:.2f}%")
# -------------------------------------------

# make a table showing counts of times in ranges of 50 cycles and print it
table = {}
for t in times:
    bucket = (t // 50) * 50
    table[bucket] = table.get(bucket, 0) + 1
print("Time Range (cycles) | Count")
print("---------------------|-------")
for k in sorted(table.keys()):
    print(f"{k:>18} - {k+49:<7} | {table[k]}")  

nr_bins = (max(times) - min(times) + 50) // 2
plt.hist(times, bins=nr_bins)
plt.savefig("hist.png")
print("Histogram saved as hist.png.")

