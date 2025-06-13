import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np
import os
import re
import argparse

plt.style.use('ggplot') # Set the default style of seaborn plots

# Message Delay parameter array (unit: microseconds µs) for ASYNC data
delay_arr_async = [100000, 10000, 1000, 100, 10] # From largest to smallest in the original Bash script

# List to store all parsed data for ASYNC plot
data_for_async_plot = []

# Helper function: Convert microseconds to readable string (µs, ms, s)
def format_time_us(us_value):
    if us_value >= 1_000_000: # 1 second = 1,000,000 microseconds
        return f"{us_value / 1_000_000:.0f}s"
    elif us_value >= 1_000: # 1 millisecond = 1,000 microseconds
        return f"{us_value / 1_000:.0f}ms"
    else:
        return f"{us_value:.0f}µs" # Microseconds

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

# Custom formatter for y-axis to display in 'k' (thousands)
def opps_formatter(x, pos):
    if x >= 1000000:
        return f'{x/1000000:.0f}M'
    if x >= 1000:
        return f'{x/1000:.0f}k'
    return f'{x:.0f}'

# --- Parse command-line arguments ---
parser = argparse.ArgumentParser(description='Plot system performance from NS-3 simulation results.')
parser.add_argument('--filepath', type=str, default='result',
                    help='Base path to the simulation results directory (e.g., "result"). This directory should contain "Async" and "SyncErr_X" folders.')
parser.add_argument('--runtime', type=int, default=1,
                    help='Total simulation run time in seconds, used for Opps calculation. Default is 5.0.')
args = parser.parse_args()

results_base_dir = args.filepath
simulation_seconds = args.runtime

# --- 2. Read ASYNC data ---
async_results_dir = os.path.join(results_base_dir, "Async")
print(f"Reading ASYNC data from '{async_results_dir}' directory...")
print(f"Using simulation duration: {simulation_seconds} seconds for Opps calculation.")

# Ensure delays are sorted for proper plotting later
sorted_delay_arr_async = sorted(delay_arr_async, reverse=True) # Sort descending for x-axis

for delay_us in sorted_delay_arr_async:
    # Construct the path to the current result directory
    current_dir = os.path.join(async_results_dir, f"Delay_{delay_us}us")

    print(f"Info: Reading data from directory '{current_dir}'...")

    num_lines = 0
    if os.path.exists(current_dir) and os.path.isdir(current_dir):
        ops_value = None

        with open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r') as f:
            num_lines = sum(1 for line in f)
            
        ops_value = num_lines / simulation_seconds

        if ops_value is not None:
            data_for_async_plot.append({
                'Delay_us': delay_us,
                'Opps': ops_value
            })
    else:
        print(f"Warning: Directory '{current_dir}' does not exist or is not a directory, skipping.")

if not data_for_async_plot:
    print("Error: No valid ASYNC simulation result data found. Please check 'results_base_dir/Async' and .dat file content.")
    # exit() # Don't exit here, as we might still plot sync reference lines

df_async = pd.DataFrame(data_for_async_plot)
print(df_async)

# --- 3. Define the y-values for the five horizontal lines (reference lines) ---
# These are the specific (Delay Bound, Sync Err) combinations to reference
# Format: (MsgDelay_str, SyncError_str)
sync_reference_points_str = [
    ('10µs', '10ns'),
    ('100µs', '100ns'),
    ('1ms', '1µs'),
    ('10ms', '10µs'),
    ('100ms', '100µs')
]

# Mapping string representations to actual nanosecond values for directory lookup
# Delay Bound for Sync results (ns)
sync_ref_delay_ns_map = {
    '10µs': 10,
    '100µs': 100,
    '1ms': 1000,
    '10ms': 10000,
    '100ms': 100000
}
# Sync Error for Sync results (ns)
sync_ref_error_ns_map = {
    '10ns': 10,
    '100ns': 100,
    '1µs': 1000,
    '10µs': 10000,
    '100µs': 100000
}

reference_lines_data = []
sync_result_dir = os.path.join(results_base_dir, "Sync")

print("\nReading SYNCHRONOUS reference data for horizontal lines...")
for delay_str, sync_err_str in sync_reference_points_str:
    delay_ns = sync_ref_delay_ns_map.get(delay_str)
    sync_err_ns = sync_ref_error_ns_map.get(sync_err_str)

    if delay_ns is None or sync_err_ns is None:
        print(f"Warning: Invalid string format for reference point ({delay_str}, {sync_err_str}), skipping.")
        continue

    current_dir = os.path.join(sync_result_dir, f"Delay_{delay_ns}us", f"Sync_{sync_err_ns}ns")

    num_lines = 0
    if os.path.exists(current_dir) and os.path.isdir(current_dir):
        with open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r') as f:
            num_lines = sum(1 for line in f)
        
        ops_value_sync = num_lines / simulation_seconds

        if not np.isnan(ops_value_sync): # Only add if a valid Opps value was found
            reference_lines_data.append({
                'label': f'Sync ({delay_str}, {sync_err_str})',
                'opps': ops_value_sync
            })
    else:
        print(f"Warning: Sync reference directory '{sync_result_dir}' does not exist or is not a directory, skipping.")

print(reference_lines_data)

# --- 4. Prepare data for plotting ---
if not df_async.empty:
    df_async['Formatted_Delay'] = df_async['Delay_us'].apply(format_time_us)
    df_async['Formatted_Delay'] = pd.Categorical(df_async['Formatted_Delay'],
                                                categories=[format_time_us(d) for d in sorted_delay_arr_async],
                                                ordered=True)
# --- 5. Plotting ---
sns.set_style("whitegrid")
plt.rcParams['font.size'] = 14
plt.rcParams['axes.labelsize'] = 16
plt.rcParams['axes.titlesize'] = 18
plt.rcParams['legend.fontsize'] = 12
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12

plt.figure(figsize=(14, 8))
ax = plt.gca() # Get current axes for adding lines and legend

# Plot ASYNC data line if available
if not df_async.empty:
    sns.lineplot(
        x='Formatted_Delay',
        y='Opps',
        data=df_async,
        marker='o',
        dashes=False,
        color='steelblue', # Assign a single color for the async line
        linewidth=2.5,
        label='Async Performance' # Label for async line in legend
    )

# --- 6. Add SYNCHRONOUS reference points and vertical markers ---
# Define distinct markers, colors, and linestyles for reference points
ref_linestyles = ['--', ':', '-.', (0, (3, 5, 1, 5)), (0, (3, 1, 1, 1))] # Custom dash patterns
ref_colors = ['red', 'green', 'purple', 'orange', 'brown'] # Different colors for distinction
ref_markers = ['^', 's', 'D', 'p', 'X'] # Markers to appear in legend, though not on the line itself

# Collect handles and labels for the combined legend
handles, labels = ax.get_legend_handles_labels() if not df_async.empty else ([], []) # Get existing handles if async plotted

for i, ref_data in enumerate(reference_lines_data):
    print(i) # Keep user's print for debugging
    print(ref_data) # Keep user's print for debugging

    # Plot a horizontal line across the entire x-axis range
    # Note: For horizontal lines, ax.get_xlim() gives the correct span
    line, = ax.plot([0,4], [ref_data['opps'], ref_data['opps']],
                     color=ref_colors[i % len(ref_colors)],
                     linestyle=ref_linestyles[i % len(ref_linestyles)],
                     linewidth=1.5,
                     marker=ref_markers[i % len(ref_markers)], # Assign marker for legend only
                     markersize=8, # Size of marker in legend
                     label=ref_data['label'])
    handles.append(line)
    labels.append(ref_data['label'])

# --- 7. Customize axis and labels ---
plt.yscale('linear')
ax.yaxis.set_major_formatter(mticker.FuncFormatter(opps_formatter))

plt.title('Operations Per Second (Opps)', fontsize=18)
plt.xlabel('End-to-End Delay', fontsize=16)
plt.ylabel('Operations Per Second (Log Scale)', fontsize=16)

plt.xticks(rotation=45, ha='right')

# Combine and display the legend
if handles:
    ax.legend(handles=handles, labels=labels, title='Reference Points', bbox_to_anchor=(1.05, 1), loc='upper left')

sns.despine(top=True, right=True)

plt.tight_layout()
plt.savefig('async-opps.pdf', dpi=300)