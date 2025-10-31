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

nr_bins = (max(times) - min(times) + 50) // 2
plt.hist(times, bins=nr_bins)
plt.savefig("hist.png")
print("Histogram saved as hist.png.")

