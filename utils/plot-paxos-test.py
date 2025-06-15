import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import re
import matplotlib.ticker as mticker
from matplotlib.ticker import FuncFormatter
import math

def format_large_numbers(value, _):
    if value >= 1_000_000_000:
        return f"{int(value // 1_000_000_000)}G"
    elif value >= 1_000_000:
        return f"{int(value // 1_000_000)}M"
    elif value >= 1_000:
        return f"{int(value // 1_000)}k"
    else:
        return f"{int(value)}"

def to_readable_time(time_str):
    match = re.match(r'(\d+\.?\d*)\s*(ms|us|ns)', time_str)
    if not match:
        raise ValueError(f"can not parse time string: {time_str}")
    
    value, unit = match.groups()
    value = float(value)
    
    # Convert to ns as the base unit
    if unit == 'ms':
        value_ns = value * 1_000_000  # 1ms = 1,000,000ns
    elif unit == 'us':
        value_ns = value * 1_000  # 1us = 1,000ns
    elif unit == 'ns':
        value_ns = value
    else:
        raise ValueError(f"can not parse time unit: {unit}")
    
    # Convert to readable time string
    if value_ns >= 1_000_000:  # ms
        return f"{int(value_ns // 1_000_000)}ms"
    elif value_ns >= 1_000:  # us
        return f"{int(value_ns // 1_000)}us"
    else:  #ns
        return f"{int(value_ns)}ns"

class PaxosPlot:
    def __init__(self, results_root_dir, simulation_seconds):
        self.results_root_dir = results_root_dir
        self.simulation_seconds = simulation_seconds
        
        print(f"Results root directory: {results_root_dir}")
        print(f"Simulation duration: {simulation_seconds} seconds for Opps calculation.")
        
        self.sync_res_dir = os.path.join(results_root_dir, "Sync")
        self.async_res_dir = os.path.join(results_root_dir, "Async")

    def plot_sync_opps(self):
        # Plot the results to a line chart
        # X-axis: delay bound
        # Y-axis: number of operations per second
        plt.figure()
        plt.yscale('log')
        sns.lineplot(data=self.sync_result_df, markers=True, markersize=10, linewidth=2.5)

        plt.xlabel("Clock Sync. Error", fontsize=14)
        plt.ylabel("Operations per second (log scale)", fontsize=14)
        plt.xticks(rotation=45, ha='right', fontsize=12)
        plt.yticks(fontsize=12)
        plt.grid(True, color='gray', linestyle='--', linewidth=0.5)
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2) 
        ax.spines['right'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        plt.legend(title="Bound Delay", loc='best', fontsize=12)
        plt.gca().yaxis.set_major_formatter(FuncFormatter(format_large_numbers))

        plt.tight_layout()

        pdf_path = os.path.join(self.results_root_dir, "sync-opps.pdf")
        plt.savefig(pdf_path, dpi=300, format='pdf', bbox_inches='tight')
        plt.close()
    
    def plot_async_opps(self):
        # Plot the results to a line chart
        # X-axis: delay bound
        # Y-axis: number of operations per second
        plt.figure()
        plt.yscale('log')
        sns.barplot(data=self.async_result_df, color='white',
                    width=0.5, edgecolor='black', hatch='x')
        
        plt.xlabel("End to End Delay", fontsize=14)
        plt.ylabel("Operations per second (log scale)", fontsize=14)
        plt.xticks(rotation=45, ha='right', fontsize=12)
        plt.yticks(fontsize=12)
        plt.grid(True, color='gray', linestyle='--', linewidth=0.5)
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2) 
        ax.spines['right'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        plt.tick_params(axis='both', colors='black')
        plt.gca().yaxis.set_major_formatter(FuncFormatter(format_large_numbers))
        
        # Plot the sync result line as comparison
        sync_results = self.sync_result_df.loc["50ns"]
        # generate different colors and line markers for each sync result line
        colors = sns.color_palette("hls", len(sync_results))
        line_styles = ['-', '--', '-.', ':', 'dashed']
        lines = []
        for i, sync_res in enumerate(sync_results):
            line = plt.axhline(y=sync_res, color=colors[i], linestyle=line_styles[i], linewidth=2.5, label=self.sync_result_df.columns[i])
            lines.append(line)
        
        plt.legend(handles=lines, loc='best', fontsize=12, title="Bound Delay")
        
        print(sync_results)

        #plt.tight_layout()
        pdf_path = os.path.join(self.results_root_dir, "async-opps.pdf")
        plt.savefig(pdf_path, dpi=300, format='pdf', bbox_inches='tight')
        

    def parse_sync_result_dir(self):
        # Parse the sync test results
        print(f"Reading sync test results from '{self.sync_res_dir}' directory...")        
        results_dict = {}
        # list dirs in this directory
        sync_dirs = os.listdir(self.sync_res_dir)
        for delay_dir in sync_dirs:
            # Parse the delay bound
            # Dir name format: Delay_50us
            delay_bound = int(re.search(r"Delay_(\d+)us", delay_dir).group(1))
            results_dict[f"{delay_bound}us"] = {}

            for sync_dir in os.listdir(os.path.join(self.sync_res_dir, delay_dir)):
                # Parse the sync error. Dir name format: Sync_100ns
                sync_err = int(re.search(r"Sync_(\d+)ns", sync_dir).group(1))
                current_dir = os.path.join(self.sync_res_dir, delay_dir, sync_dir)
                
                # Parse the number of operations. Get the number of lines of the first file in the directory
                num_lines = sum(1 for line in open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r'))
            
                # Calculate the number of operations per second
                opps = num_lines / self.simulation_seconds
                results_dict[f"{delay_bound}us"][f"{sync_err}ns"] = opps
        
        # Transform the dictionary to a Pandas DataFrame
        self.sync_result_df = pd.DataFrame.from_dict(results_dict).sort_index(axis=0).sort_index(axis=1)
        self.sync_result_df = self.sync_result_df.rename(columns=to_readable_time, index=to_readable_time)
        print(f"Sync test results:\n{self.sync_result_df}")
        print(self.sync_result_df.describe())
    
    def parse_async_result_dir(self):
        # Parse the async test results
        print(f"Reading async test results from '{self.async_res_dir}' directory...")        
        results_dict = {}
        # list dirs in this directory
        async_dirs = os.listdir(self.async_res_dir)
        for delay_dir in async_dirs:
            current_dir = os.path.join(self.async_res_dir, delay_dir)
            # Parse the e2e delay. Dir name format: Delay_50us
            e2e_delay = int(re.search(r"Delay_(\d+)us", delay_dir).group(1))
            
            # Parse the number of operations. Get the number of lines of the first file in the directory
            num_lines = sum(1 for line in open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r'))
            
            # Calculate the number of operations per second
            opps = num_lines / self.simulation_seconds

            results_dict[f"{e2e_delay}us"] = opps
        
        print(f"Async test results:\n{results_dict}")
            
        # Transform the dictionary to a Pandas DataFrame
        self.async_result_df = pd.DataFrame(results_dict, index=[0]).sort_index(axis=1)
        self.async_result_df = self.async_result_df.rename(columns=to_readable_time)
        print(f"Async test results:\n{self.async_result_df}")
    
if __name__ == "__main__":
    # Parse the command line arguments
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--results-dir", type=str, default="result", help="Base directory for the results.")
    parser.add_argument("--runtime", type=int, default=5, help="Simulation runtime in seconds.")
    args = parser.parse_args()
    
    # Create a PaxosPlot object
    plot = PaxosPlot(args.results_dir, args.runtime)
    plot.parse_sync_result_dir()
    plot.parse_async_result_dir()
    plot.plot_sync_opps()
    plot.plot_async_opps()
