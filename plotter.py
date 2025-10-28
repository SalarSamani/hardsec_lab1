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
threshold = 65
below = sum(1 for t in times if t <= threshold)
above = len(times) - below
print(f"<= {threshold}: {below}, > {threshold}: {above}")
print(f"{below/above:.2f} times more below than above {threshold} cycles.")
# -------------------------------------------

nr_bins = (max(times) - min(times) + 50) // 2
plt.hist(times, bins=nr_bins)
plt.savefig("hist.png")
print("Histogram saved as hist.png.")

