
import os
import re
import matplotlib.pyplot as plt
import seaborn as sns

sync_data_file = "data/paxos/sync-paxos-test.csv"
async_data_file = "data/paxos/async-paxos-test.csv"

sync_data = []
async_data = []

with open(sync_data_file, "r") as f:
    for line in f:
        sync_data.append(float(line))
        
with open(async_data_file, "r") as f:
    for line in f:
        async_data.append(float(line))

sync_delay = [x * 1e6 for x in sync_data]
async_delay = [x * 1e6 for x in async_data]

print("Sync. Paxos delay max:", max(sync_delay))
print("Async. Paxos delay max:", max(async_delay))

# Get the median value
sync_median = sorted(sync_delay)[len(sync_delay) // 2]
async_median = sorted(async_delay)[len(async_delay) // 2]

print("Sync. Paxos median:", sync_median)
print("Async. Paxos median:", async_median)
print("Ratio:", sync_median / async_median)

# Plot the CDF in one figure
sns.set_style(style="whitegrid")

plt.figure(figsize=(3, 2.5))
plt.xscale("log")
plt.xlabel("(a) Consensus Time (us)", fontsize=14)
plt.ylabel("CDF", fontsize=14)
plt.grid(True, linestyle='--', linewidth=0.5, color='gray')

# Second to microsecond
plt.tight_layout()
sns.ecdfplot(data=sync_delay, label="Sync. Paxos", linewidth=2.5)
sns.ecdfplot(data=async_delay, label="Async. Paxos", linewidth=2.5)
plt.legend()
plt.savefig("paxos_delay.pdf")
