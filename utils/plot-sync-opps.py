import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import re
import matplotlib.ticker as mticker # Import for custom formatter

# 1. Define parameter arrays, consistent with your Bash script
# Clock synchronization error parameter array (unit: nanoseconds ns)
delay_bound_arr = [100000, 10000, 1000, 100, 10]    #unit: us
sync_error_arr = [100000, 10000, 1000, 100, 10]     #unit: ns
# Message delay bound parameter array (unit: nanoseconds ns)

# Root directory for results
results_root_dir = "result/Sync"

# List to store all parsed data
data_for_plot = []

# Helper function: Convert nanoseconds to readable string (ns, µs, ms, s)
def format_time_ns(ns_value):
    if ns_value >= 1_000_000_000:
        return f"{ns_value / 1_000_000_000:.0f}s"
    elif ns_value >= 1_000_000:
        return f"{ns_value / 1_000_000:.0f}ms"
    elif ns_value >= 1_000:
        return f"{ns_value / 1_000:.0f}µs"
    else:
        return f"{ns_value:.0f}ns"

# 2. Iterate through result directories and read data
print(f"Reading data from '{results_root_dir}' directory...")
for msg_delay in delay_bound_arr:
    for sync_err in sync_error_arr:
        # Construct the path to the current result directory
        current_dir = os.path.join(results_root_dir, f"Delay_{msg_delay}us", f"Sync_{sync_err}ns")

        print(f"Info: Reading data from directory '{current_dir}'...")
        
        # Get the number of lines of the first file in the directory
        num_lines = 0
        if os.path.exists(current_dir) and os.path.isdir(current_dir):
            # get one of the files in the directory and count number of lines
            with open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r') as f:
                num_lines = sum(1 for line in f)
            
            ops_value = num_lines

            # If Ops value was successfully obtained, add it to the data list
            if ops_value is not None:
                data_for_plot.append({
                    'SyncError_ns': sync_err,
                    'MsgDelay_ns': msg_delay,
                    'Opps': ops_value
                })
        else:
            print(f"Warning: Directory '{current_dir}' does not exist or is not a directory, skipping.")

if not data_for_plot:
    print("Error: No valid simulation result data found. Please check 'result' directory structure and .dat file content.")
    exit()

# 3. Convert data to Pandas DataFrame
df = pd.DataFrame(data_for_plot)

# 4. Create readable labels
df['Formatted_MsgDelay'] = df['MsgDelay_ns'].apply(format_time_ns)
df['Formatted_SyncError'] = df['SyncError_ns'].apply(format_time_ns)

# Sort 'Formatted_MsgDelay' for the x-axis to ensure correct order
# Changed to sort in descending order based on original delay_bound_arr values
df['Formatted_MsgDelay'] = pd.Categorical(df['Formatted_MsgDelay'],
                                          categories=[format_time_ns(d) for d in sorted(delay_bound_arr, reverse=True)],
                                          ordered=True)

# Sort 'Formatted_SyncError' for the hue axis to ensure consistent legend and bar order
# Changed to sort in ascending order based on original sync_error_arr values
df['Formatted_SyncError'] = pd.Categorical(df['Formatted_SyncError'],
                                           categories=[format_time_ns(s) for s in sorted(sync_error_arr, reverse=True)],
                                           ordered=True)

# 5. Plot the grouped bar chart
plt.figure(figsize=(12, 7)) # Set chart size
sns.barplot(x='Formatted_MsgDelay', y='Opps', hue='Formatted_SyncError', data=df, palette='viridis')

plt.yscale('log') # Set y-axis to logarithmic scale

# Custom formatter for y-axis to display in 'k' (thousands)
def opps_formatter(x, pos):
    if x >= 1000_000:
        return f'{x/1000_000:.0f}M' # If value is 1000000 or more, divide by 1000000 and append 'M'
    if x >= 1000:
        return f'{x/1000:.0f}k' # If value is 1000 or more, divide by 1000 and append 'k'
    return f'{x:.0f}' # Otherwise, display as is

plt.gca().yaxis.set_major_formatter(mticker.FuncFormatter(opps_formatter))

sns.set_style("whitegrid") # Set the overall style to whitegrid
plt.rcParams['font.size'] = 14 # Set global font size, increased from 12
plt.rcParams['axes.labelsize'] = 27 # Set axis label font size, increased from 14
plt.rcParams['axes.titlesize'] = 27 # Set title font size, increased from 16
plt.rcParams['legend.fontsize'] = 12 # Set legend font size, increased from 10
plt.rcParams['xtick.labelsize'] = 18 # Set x-tick label font size, increased from 10
plt.rcParams['ytick.labelsize'] = 18 # Set y-tick label font size, increased from 10


# Add title and axis labels
#plt.title('Different Message Delay and Clock Sync Error vs. Operations Per Second (Opps)', fontsize=16)
plt.xlabel('Bounded Delay', fontsize=12)
plt.ylabel('Operations Per Second (Log Scale)', fontsize=12) # Update y-axis label
plt.xticks(rotation=45, ha='right') # Rotate x-axis labels for readability
plt.legend(title='Sync Error', bbox_to_anchor=(1.05, 1), loc='upper left') # Place legend outside the plot
plt.grid(axis='y', linestyle='--', alpha=0.7) # Add grid lines

plt.tight_layout() # Automatically adjust layout to prevent labels from overlapping

# Optional: Save the chart
plt.savefig('paxos_performance_grouped_bar_chart.pdf', dpi=300, bbox_inches='tight')
print("Chart saved as 'paxos_performance_grouped_bar_chart.pdf'")