import os
import re
import logging
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter
import seaborn as sns

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

# Function to parse time to nanoseconds for sorting
def parse_time_to_ns(time_str):
    match = re.match(r'(\d+\.?\d*)\s*(ms|us|ns)', time_str)
    if not match:
        raise ValueError(f"error: {time_str}")
    
    value, unit = match.groups()
    value = float(value)
    
    # Convert to ns as the base unit
    if unit == 'ms':
        value_ns = value * 1_000_000
    elif unit == 'us':
        value_ns = value * 1_000
    elif unit == 'ns':
        value_ns = value
    else:
        raise ValueError(f"error : {unit}")
    
    return value_ns

class SyncDCPlot:
    def __init__(self, root_dir, paxos_dir, paxos_runtime, allreduce_dir, allreduce_workload, ring_path, fastpass_path, ocs_path, ocs_delay_path):
        self.root_dir               = root_dir
        self.paxos_runtime          = paxos_runtime
        self.paxos_data_dir         = os.path.join(self.root_dir, paxos_dir)
        self.paxos_data_sync_dir    = os.path.join(self.paxos_data_dir, "sync")
        self.paxos_data_async_dir   = os.path.join(self.paxos_data_dir, "async")

        self.allreduce_data_dir     = allreduce_dir
        self.allreduce_workload     = allreduce_workload
        self.ring_path              = ring_path
        self.fastpass_path          = fastpass_path
        self.ocs_path               = ocs_path
        self.ocs_delay_path         = ocs_delay_path
        
    def parse_paxos_sync_result_dir(self):
        # Parse the sync test results
        logging.info(f"Reading sync test results from {self.root_dir}")
        opps_dict = {}
        # list dirs in this directory
        for delay_dir in os.listdir(self.paxos_data_sync_dir):
            # Parse delay bound, dir name: delay_50us
            delay_bound = int(re.search(r"delay_(\d+)us", delay_dir).group(1))
            opps_dict[f"{delay_bound}us"] = {}
            #traffic_dict[delay_bound] = {}
            # list dirs in this directory
            for sync_dir in os.listdir(os.path.join(self.paxos_data_sync_dir, delay_dir)):
                # Parse sync error, dir name: sync_100ns
                sync_err = int(re.search(r"sync_(\d+)ns", sync_dir).group(1))
                curr_dir = os.path.join(self.paxos_data_sync_dir, delay_dir, sync_dir)
                
                # Parse the number of operations per second
                file = os.listdir(curr_dir)[0]
                num_lines = len(open(os.path.join(curr_dir, file), 'r').readlines())
                
                opps = (num_lines - 2) / self.paxos_runtime
                opps_dict[f"{delay_bound}us"][f"{sync_err}ns"] = opps
                
                # Parse packets number of each server
                #server_packets = {}
                #for file in os.listdir(curr_dir):
                #    server_id = int(file.split('-')[1])
                #    lines = open(os.path.join(curr_dir, file), 'r').readlines()
                #    send_packets = int(lines[-2].split(' ')[-1])
                #    recv_packets = int(lines[-1].split(' ')[-1])
                #    server_packets[server_id] = (send_packets, recv_packets)
                #traffic_dict[delay_bound][sync_err] = server_packets
        
        # Transform the dictionary to a Pandas DataFrame
        self.paxos_sync_opps_df = pd.DataFrame.from_dict(opps_dict).sort_index(axis=0).sort_index(axis=1)
        #self.paxos_sync_result_df.index.attrs["name"] = "Delay Bound (us)"
        #self.paxos_sync_result_df.columns.attrs["name"] = "Sync Error (ns)"
        print(self.paxos_sync_opps_df.describe())
        
        #self.paxos_sync_traffic_dict = traffic_dict
        delay_filename = os.path.join(self.paxos_data_dir, "sync-paxos-test.csv")
        self.paxos_sync_delay = pd.read_csv(delay_filename)
        print(self.paxos_sync_delay)
    
    def parse_paxos_async_result_dir(self):
        # Parse the async test results
        logging.info(f"Reading async test results from {self.root_dir}")
        opps_dict = {}
        # list dirs in this directory
        for delay_dir in os.listdir(self.paxos_data_async_dir):
            current_dir = os.path.join(self.paxos_data_async_dir, delay_dir)
            e2e_delay = int(re.search(r"delay_(\d+)us", delay_dir).group(1))
            
            num_lines = sum(1 for line in open(os.path.join(current_dir, os.listdir(current_dir)[0]), 'r'))
            
            # calculate the number of operations per second
            opps = num_lines / self.paxos_runtime
        
            opps_dict[f"{e2e_delay}us"] = opps
        
        # Transform the dictionary to a Pandas DataFrame
        self.paxos_async_opps_df = pd.DataFrame(opps_dict, index=[0]).sort_index(axis=1)
        
        delay_filename = os.path.join(self.paxos_data_dir, "async-paxos-test.csv")
        self.paxos_async_delay = pd.read_csv(delay_filename)
        
        print(self.paxos_async_opps_df.describe())
    
    def parse_allreduce_workload(self):
        logging.info(f"Parsing workload file: {self.allreduce_workload}")
        workload_file = os.path.join(self.root_dir, self.allreduce_data_dir, self.allreduce_workload)
        if not os.path.exists(workload_file):
            logging.error(f"Workload file not found: {workload_file}")
            exit(1)
        # Parse workload data: job_id, request_servers, create_time, message_size
        df = pd.read_csv(workload_file, sep=',')
        self.allreduce_workload_df = df
        logging.info(df.describe())
    
    def parse_allreduce_ring_data(self):
        logging.info(f"Parsing ring data file: {self.ring_path}")
        ring_file_dir = os.path.join(self.root_dir, self.allreduce_data_dir, self.ring_path)
        if not os.path.exists(ring_file_dir):
            logging.error(f"Ring data file not found: {ring_file_dir}")
            exit(1)
        # Parse ring data, list all files in the directory and pasre each file
        ring_files = os.listdir(ring_file_dir)
        
        # Classify two types of files: jobs completed time(*.log) and  packet delay(*.delay)
        jobs_files = [file for file in ring_files if file.endswith('.log')]
        delay_files = [file for file in ring_files if file.endswith('.delay')]
        
        jobs_data = {}
        for job_file in jobs_files:
            # File format: job_1.log
            job_id = int(job_file.split('_')[1].split('.')[0])
            with open(os.path.join(ring_file_dir, job_file), 'r') as f:
                start_time = float(f.readline().split(',')[0])
                end_time = float(f.readlines()[-1].split(',')[0])
            jobs_data[job_id] = [start_time, end_time]
        
        # Convert to dataframe [job_id, start_time, end_time]
        df = pd.DataFrame([(job_id, times[0], times[1]) for job_id, times in jobs_data.items()], columns=['job_id', 'start_time', 'end_time'])
        self.allreduce_ring_job_df = df
        logging.info(self.allreduce_ring_job_df.describe())
        
        delay_list = []
        for delay_file in delay_files:
            with open(os.path.join(ring_file_dir, delay_file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
        
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.allreduce_ring_delay_df = df
        logging.info(self.allreduce_ring_delay_df.describe())
    
    def parse_allreduce_fastpass_data(self):
        logging.info(f"Parsing fastpass data file: {self.fastpass_path}")
        fastpass_file_dir = os.path.join(self.root_dir, self.allreduce_data_dir, self.fastpass_path)
        if not os.path.exists(fastpass_file_dir):
            logging.error(f"Fastpass data file not found: {fastpass_file_dir}")
            exit(1)
            
        # Parse fastpass data, list all files in the directory and pasre each file
        fastpass_files = os.listdir(fastpass_file_dir)
        
        # Classify two types of files: jobs completed time(*.log) and packet delay(*.delay)
        jobs_files = [file for file in fastpass_files if file.endswith('.log')]
        delay_files = [file for file in fastpass_files if file.endswith('.delay')]
        
        fastpass_data = {}
        for job_file in jobs_files:
            job_id = int(job_file.split('_')[1].split('.')[0])
            with open(os.path.join(fastpass_file_dir, job_file), 'r') as f:
                start_time = float(f.readline().split(',')[0])
                end_time = float(f.readlines()[-1].split(',')[0])
            fastpass_data[job_id] = [start_time, end_time]
            
        # Convert to dataframe
        df = pd.DataFrame([(job_id, times[0], times[1]) for job_id, times in fastpass_data.items()], columns=['job_id', 'start_time', 'end_time'])
        self.allreduce_fastpass_job_df = df
        logging.info(self.allreduce_fastpass_job_df.describe())
        
        delay_list = []
        for delay_file in delay_files:
            with open(os.path.join(fastpass_file_dir, delay_file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
        
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.allreduce_fastpass_delay_df = df
        logging.info(self.allreduce_fastpass_delay_df.describe())
    
    def parse_allreduce_ocs_data(self):
        logging.info(f"Parsing OCS data file: {self.ocs_path}")
        ocs_file = os.path.join(self.root_dir, self.allreduce_data_dir, self.ocs_path)
        if not os.path.join(self.root_dir, ocs_file):
            logging.error(f"OCS data file not found: {ocs_file}")
            return None
        
        # Read CSV file: job_id, start_time, end_time
        df = pd.read_csv(ocs_file, sep=',')
        self.allreduce_ocs_job_df = df
        logging.info(self.allreduce_ocs_job_df.describe())
        
        # Parse OCS delay
        ocs_delay_dir = os.path.join(self.root_dir, self.allreduce_data_dir, self.ocs_delay_path)
        if not os.path.exists(ocs_delay_dir):
            logging.error(f"OCS delay directory not found: {ocs_delay_dir}")
            exit(1)
        
        delay_files = os.listdir(ocs_delay_dir)
        delay_list = []
        for file in delay_files:
            with open(os.path.join(ocs_delay_dir, file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
                
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.allreduce_ocs_delay_df = df
        logging.info(self.allreduce_ocs_delay_df.describe())
    
    def plot_allreduce_evaluation(self):
        # Plot all-in-one
        # Subplot 1x2
        fig, axes = plt.subplots(1, 2, figsize=(7, 2.5))
        colors = sns.color_palette('husl', 5)
        sns.set_style('whitegrid')        
        self.plot_allreduce_completion_time(axes[0], colors)
        self.plot_allreduce_packet_delay(axes[1], colors)

        # savefig 
        plt.tight_layout()
        plt.savefig('allreduce_evalution.pdf', bbox_inches='tight')

    def plot_paxos_traffic_bar(self, ax, colors):
        pass

    def plot_paxos_delay_comparison(self):
        fig = plt.figure(figsize=(3.5, 2.5))
        sns.set_style('whitegrid')

        # seconds to microseconds
        sync_delay = self.paxos_sync_delay * 1e6
        async_delay = self.paxos_async_delay * 1e6
        
        colors = sns.color_palette('husl', 2)

        # Plot CDF
        sns.ecdfplot(data=sync_delay, label='Sync. Paxos', linewidth=2.5, color=colors[0])
        sns.ecdfplot(data=async_delay, label='Async. Paxos', linewidth=2.5, color=colors[1])
        plt.grid(True, linestyle='--', linewidth=0.5, color='gray')

        sync_delay_max = sync_delay.max()
        async_delay_max = async_delay.max()

        plt.xlabel("Consensus Time (us)")
        plt.ylabel("CDF")
        plt.xscale('log')
        plt.legend()
        plt.savefig('paxos_delay.pdf')
        plt.close()

    def plot_paxos_sync_line(self):
        df = self.paxos_sync_opps_df
        # Convert index to readable format
        df.index = [to_readable_time(idx) for idx in df.index]
        df.columns = [to_readable_time(column) for column in df.columns]
        
        # Sort index by numerical value (in nanoseconds)
        index_ns = {idx: parse_time_to_ns(idx) for idx in df.index}
        sorted_index = sorted(index_ns, key=lambda x: index_ns[x])
        df = df.loc[sorted_index]
        print(df)
        
        # Sort columns by numerical value (in nanoseconds)
        columns_ns = {col: parse_time_to_ns(col) for col in df.columns}
        sorted_columns = sorted(columns_ns, key=lambda x: columns_ns[x])
        df = df[sorted_columns]
        
        # Create figure
        plt.figure(figsize=(3.5,2.5))
        sns.set_style('whitegrid')

        # Create a line plot
        markers = ['o', 's', '^', 'D', 'v']
        i = 0
        for column in df.columns:
            sns.lineplot(x=df.index, y=df[column], label=column, marker=markers[i], linewidth=2.5)
            i = i+1

        plt.xlabel("Clock Sync. Error")
        plt.ylabel("Operations per second")
        plt.yscale('log')
        plt.legend(title='Delay Bound')
        plt.grid(True, linestyle='--', linewidth=0.5, color='gray')
        plt.savefig('paxos_sync.pdf')
        plt.close()
    
    def plot_allreduce_completion_time(self, ax, colors):
        # calculate job completion time
        ring_df     = pd.merge(self.allreduce_workload_df, self.allreduce_ring_job_df, on='job_id', how='left')
        fastpass_df = pd.merge(self.allreduce_workload_df, self.allreduce_fastpass_job_df, on='job_id', how='left')
        ocs_df      = pd.merge(self.allreduce_workload_df, self.allreduce_ocs_job_df, on='job_id', how='left')

        # plot total time CDF end_time - create_time, ns to ms
        ring_plot_ms     = (ring_df['end_time'] - ring_df['create_time']) / 1e6
        fastpass_plot_ms = (fastpass_df['end_time'] - fastpass_df['create_time']) / 1e6
        ocs_plot_ms      = (ocs_df['end_time'] - ocs_df['create_time']) / 1e6

        sns.ecdfplot(data=ring_plot_ms, label='Ring', ax=ax, linewidth=2.5)
        sns.ecdfplot(data=fastpass_plot_ms, label='Fastpass', ax=ax, linewidth=2.5)
        sns.ecdfplot(data=ocs_plot_ms, label='OCS', ax=ax, linewidth=2.5)
        
        # Add vertical lines at the maximum value of each CDF
        max_ring = ring_plot_ms.max()
        max_fastpass = fastpass_plot_ms.max()
        max_ocs = ocs_plot_ms.max()

        ax.axvline(x=max_ring, color='gray', linestyle='--', alpha=0.5)
        ax.axvline(x=max_fastpass, color='gray', linestyle='--', alpha=0.5)
        ax.axvline(x=max_ocs, color='gray', linestyle='--', alpha=0.5)
        
        print(max_ocs/max_ring)
        print(max_ocs/max_fastpass)
        
        ax.set_ylabel('CDF', fontsize=13)
        ax.set_xlabel('(a) Allreduce Completion Time (ms)', fontsize=13)
        ax.legend(bbox_to_anchor=(0.4, 0.5))
        ax.grid(True, linestyle='--', linewidth=0.5, color='gray')

    def plot_allreduce_packet_delay(self, ax, colors):
        ring_delay_data = self.allreduce_ring_delay_df['delay'] / 1e3
        fastpass_delay_data = self.allreduce_fastpass_delay_df['delay'] / 1e3
        ocs_delay_data = self.allreduce_ocs_delay_df['delay'] / 1e3 + 2
        
        sns.ecdfplot(data=ring_delay_data, label='Ring', ax=ax, linewidth=2.5)
        sns.ecdfplot(data=fastpass_delay_data, label='Fastpass', ax=ax, linewidth=2.5)
        sns.ecdfplot(data=ocs_delay_data, label='OCS', ax=ax, linewidth=2.5)
        
        ax.set_xscale('log')
        ax.set_ylabel('CDF', fontsize=13)
        ax.set_xlabel('(b) Packet Delay (us)', fontsize=13)
        ax.legend()
        ax.grid(True, linestyle='--', linewidth=0.5, color='gray')
        ax.set_xticks([1e-1, 1e0, 1e1, 1e2, 1e3, 1e4])
        ax.axvline(x=1, color='gray', linestyle='--', label='Fabric Delay')
        ax.text(0.95, 0.5, 'Fabric Delay', color='gray', fontsize=10, va='center', ha='right', rotation=90)

def main():
    # Set up logging
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    logger = logging.getLogger(__name__)

    # Parse command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--data-dir', type=str, default='data', help='Path to the results directory')
    parser.add_argument('--paxos-dir', type=str, default='paxos', help='Path to the paxos results directory')
    parser.add_argument('--paxos-runtime', type=int, default=5, help="Paxos Simulation runtime in seconds.")
    parser.add_argument('--allreduce-dir', type=str, default='allreduce', help='Path to the allreduce results directory')
    parser.add_argument('--allreduce-workload', type=str, default='workload.csv', help='Allreduce workload file.')
    parser.add_argument('--ring-path', type=str, default='ring')
    parser.add_argument('--fastpass-path', type=str, default='fastpass')
    parser.add_argument('--ocs-path', type=str, default='ocs-allreduce.csv')
    parser.add_argument('--ocs-delay-path', type=str, default='ocs')
    args = parser.parse_args()

    sync_dc_plot = SyncDCPlot(args.data_dir, args.paxos_dir, args.paxos_runtime, args.allreduce_dir, args.allreduce_workload, args.ring_path, args.fastpass_path, args.ocs_path, args.ocs_delay_path)
    sync_dc_plot.parse_paxos_sync_result_dir()
    sync_dc_plot.parse_paxos_async_result_dir()
    sync_dc_plot.parse_allreduce_workload()
    sync_dc_plot.parse_allreduce_ring_data()
    sync_dc_plot.parse_allreduce_fastpass_data()
    sync_dc_plot.parse_allreduce_ocs_data()
    sync_dc_plot.plot_paxos_delay_comparison()
    sync_dc_plot.plot_paxos_sync_line()
    sync_dc_plot.plot_allreduce_evaluation()

if __name__ == '__main__':
    main()
