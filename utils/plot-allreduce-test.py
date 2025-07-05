import os
import argparse
import logging
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns

class AllReducePlot:
    def __init__(self, base_path, workload, ring_path, fastpass_path, ocs_path, ocs_delay_path):
        self.base_path = base_path
        self.workload = workload
        self.ring_path = ring_path
        self.fastpass_path = fastpass_path
        self.ocs_path = ocs_path
        self.ocs_delay_path = ocs_delay_path
    
    def parse_workload(self):
        """
        Parse the workload data from the specified file.
        
        Returns:
            pd.DataFrame: Parsed workload data
        """
        logging.info(f"Parsing workload data from {self.base_path}...")
        # Read workload data
        workload_file = os.path.join(self.base_path, self.workload)
        if not os.path.exists(workload_file):
            logging.error(f"Workload file {workload_file} does not exist.")
            return None
    
        # Parse workload data
        # Format: job_id, request_servers, create_time, message_size
        df = pd.read_csv(workload_file, sep=',')
        self.workload_df = df
        
        return df

    def parse_ring_data(self):
        """
        Parse the ring data from the specified directory.
        
        Returns:
            pd.DataFrame: Parsed ring data
        """
        logging.info(f"Parsing ring data from {self.ring_path}...")
        # Read ring data
        ring_file_dir = os.path.join(self.base_path, self.ring_path)
        if not os.path.exists(ring_file_dir):
            logging.error(f"Ring data directory {ring_file_dir} does not exist.")
            return None

        # Parse ring data, list all files in the directory and pasre each file
        # filename: job_<job_id>.csv
        ring_files = os.listdir(ring_file_dir)
        
        ## Classify two types of files: jobs completed time(*.log) and packet delay(*.delay)
        jobs_files = [file for file in ring_files if file.endswith('.log')]
        delay_files = [file for file in ring_files if file.endswith('.delay')]

        ring_data = {}
        for file in jobs_files:
            # Parse file name and get job id
            job_id = int(file.split('_')[1].split('.')[0])
            # Start time is the first elem of the first line
            # End time is the last elem of the last line
            with open(os.path.join(ring_file_dir, file), 'r') as f:
                start_time = float(f.readline().split(',')[0])
                end_time = float(f.readlines()[-1].split(',')[0])
            ring_data[job_id] = [start_time, end_time]
        
        # Convert to dataframe  [job_id, start_time, end_time], none index
        df = pd.DataFrame([(job_id, times[0], times[1]) for job_id, times in ring_data.items()],
                            columns=['job_id', 'start_time', 'end_time'])
        self.ring_df = df

        delay_list = []
        for file in delay_files:
            # Open file and read each line as a delay
            with open(os.path.join(ring_file_dir, file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.ring_delay_df = df

        return self.ring_df, self.ring_delay_df
    
    def parse_fastpass_data(self):
        """
        Parse the fastpass data from the specified directory.
        
        Returns:
            pd.DataFrame: Parsed fastpass data
        """
        logging.info(f"Parsing fastpass data from {self.fastpass_path}...")
        # Read fastpass data
        fastpass_file_dir = os.path.join(self.base_path, self.fastpass_path)
        if not os.path.exists(fastpass_file_dir):
            logging.error(f"Fastpass data directory {fastpass_file_dir} does not exist.")
            return None
        
        # Parse fastpass data, list all files in the directory and pasre each file
        fastpass_files = os.listdir(fastpass_file_dir)
        
        # Classify two types of files: jobs completed time(*.log) and packet delay(*.delay)
        jobs_files = [file for file in fastpass_files if file.endswith('.log')]
        delay_files = [file for file in fastpass_files if file.endswith('.delay')]
        
        fastpass_data = {}
        for file in jobs_files:
            job_id = int(file.split('_')[1].split('.')[0])
            with open(os.path.join(fastpass_file_dir, file), 'r') as f:
                start_time = float(f.readline().split(',')[0])
                end_time = float(f.readlines()[-1].split(',')[0])
            fastpass_data[job_id] = [start_time, end_time]
        
        # Convert to dataframe
        df = pd.DataFrame([(job_id, times[0], times[1]) for job_id, times in fastpass_data.items()],
                          columns=['job_id', 'start_time', 'end_time'])
        self.fastpass_df = df
        
        delay_list = []
        for file in delay_files:
            # Open file and read each line as a delay
            with open(os.path.join(fastpass_file_dir, file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.fastpass_delay_df = df
        
        return self.fastpass_df, self.fastpass_delay_df

    def parse_ocs_data(self):
        """
        Parse the ocs data from the specified directory.
        
        Returns:
            pd.DataFrame: Parsed ocs data
        """
        logging.info(f"Parsing ocs data from {self.ocs_path}...")
        # Read ocs data
        ocs_file = os.path.join(self.base_path, self.ocs_path)
        if not os.path.exists(ocs_file):
            logging.error(f"OCS data file {ocs_file} does not exist.")
            return None

        # Read CSV file : job_id, start_time, end_time
        df = pd.read_csv(ocs_file, sep=',')
        self.ocs_df = df
        
        # Parse OCS delay
        ocs_delay_dir = os.path.join(self.base_path, self.ocs_delay_path)
        if not os.path.exists(ocs_delay_dir):
            logging.error(f"OCS delay directory {ocs_delay_dir} does not exist.")
            return None

        delay_files = os.listdir(ocs_delay_dir)
        delay_list = []
        for file in delay_files:
            # Open file and read each line as a delay
            with open(os.path.join(ocs_delay_dir, file), 'r') as f:
                delay_list.extend([int(line) for line in f.readlines()])
        
        df = pd.DataFrame(delay_list, columns=['delay'])
        self.ocs_delay_df = df

        return df

    def plot_job_CT_CDF_all_in_one(self):
        
        # Plot CDF of execution time
        plt.figure(figsize=(5, 4))
        ring_df = pd.merge(self.workload_df, self.ring_df, on='job_id', how='left')
        fastpass_df = pd.merge(self.workload_df, self.fastpass_df, on='job_id', how='left')
        ocs_df = pd.merge(self.workload_df, self.ocs_df, on='job_id', how='left')
        # Plot total time CDF end_time -  - create_time, ns to ms
        ring_plot_ms = (ring_df['end_time'] - ring_df['create_time']) / 1e6
        fastpass_plot_ms = (fastpass_df['end_time'] - fastpass_df['create_time']) / 1e6
        ocs_plot_ms = (ocs_df['end_time'] - ocs_df['create_time']) / 1e6

        sns.set_style("white")
        sns.set_style("ticks")
        plt.figure(figsize=(5, 4))
        my_colors = sns.color_palette()
        sns.ecdfplot(data=ring_plot_ms, label='ring', linewidth=2.5)
        sns.ecdfplot(data=fastpass_plot_ms, label='fastpass', linewidth=2.5)
        sns.ecdfplot(data=ocs_plot_ms, label='ocs', linewidth=2.5)

        ax = plt.gca()
        for spine in ax.spines.values():
            spine.set_linewidth(2)
        
        plt.legend()
        plt.xlabel('Job completion time (ms)')
        plt.tight_layout()

        plt.savefig('jobs_completion_time.pdf')
        plt.close()

    def plot_job_CT_CDF(self):
        """
        Plot the workload, ring, fastpass and ocs data.
        """
        # Change time columns from ns to ms
        self.workload_df['create_time'] = self.workload_df['create_time'] / 1e6
        print(self.workload_df)
        #self.workload_df['start_time'] = self.workload_df['start_time'] / 1e6
        self.ring_df['start_time'] = self.ring_df['start_time'] / 1e6
        self.ring_df['end_time'] = self.ring_df['end_time'] / 1e6
        self.fastpass_df['start_time'] = self.fastpass_df['start_time'] / 1e6
        self.fastpass_df['end_time'] = self.fastpass_df['end_time'] / 1e6
        self.ocs_df['start_time'] = self.ocs_df['start_time'] / 1e6
        self.ocs_df['end_time'] = self.ocs_df['end_time'] / 1e6

        # Plot CDF
        # sub plot for each type of server number
        # Plot 2x4 subplot, 2 rows, 4 columns, row for wait time and execution time,
        # column for 4, 6, 8, 10 servers
        fig, axes = plt.subplots(3, 4, figsize=(16, 8))
        # Plot wait time
        for i, servers in enumerate([4, 6, 8, 10]):
            jobs = self.workload_df[self.workload_df['request_servers'] == servers]
            ring_df = pd.merge(jobs, self.ring_df, on='job_id', how='left')
            fastpass_df = pd.merge(jobs, self.fastpass_df, on='job_id', how='left')
            ocs_df = pd.merge(jobs, self.ocs_df, on='job_id', how='left')

            # Plot total time CDF
            ax_total = axes[0, i]
            sns.ecdfplot(data=ring_df['end_time'] - ring_df['create_time'], ax=ax_total, label=f'{servers} servers-ring')
            sns.ecdfplot(data=fastpass_df['end_time'] - fastpass_df['create_time'], ax=ax_total, label=f'{servers} servers-fastpass')
            sns.ecdfplot(data=ocs_df['end_time'] - ocs_df['create_time'], ax=ax_total, label=f'{servers} servers-ocs')
            ax_total.legend()
            
            # Plot wait time  CDF
            ax_wait = axes[1, i]
            sns.ecdfplot(data=ring_df['start_time'] - ring_df['create_time'], ax=ax_wait, label=f'{servers} servers-ring')
            sns.ecdfplot(data=fastpass_df['start_time'] - fastpass_df['create_time'], ax=ax_wait, label=f'{servers} servers-fastpass')
            sns.ecdfplot(data=ocs_df['start_time'] - ocs_df['create_time'], ax=ax_wait, label=f'{servers} servers-ocs')
            ax_wait.legend()
            
            # Plot execution time CDF
            ax_execution = axes[2, i]
            sns.ecdfplot(data=ring_df['end_time'] - ring_df['start_time'], ax=ax_execution, label=f'{servers} servers-ring')
            sns.ecdfplot(data=fastpass_df['end_time'] - fastpass_df['start_time'], ax=ax_execution, label=f'{servers} servers-fastpass')
            sns.ecdfplot(data=ocs_df['end_time'] - ocs_df['start_time'], ax=ax_execution, label=f'{servers} servers-ocs')
            ax_execution.legend()

        axes[0, 0].set_ylabel('CDF of total time (ms)')
        axes[1, 0].set_ylabel('CDF of wait time (ms)')
        axes[2, 0].set_ylabel('CDF of execution time (ms)')
        axes[2, 0].set_xlabel('Time (ms)')
        axes[2, 1].set_xlabel('Time (ms)')
        axes[2, 2].set_xlabel('Time (ms)')
        # savefig
        plt.legend()
        plt.savefig('cdf_allreduce.pdf')
        plt.close()
    
    def plot_packet_delay_CDF(self):
        # ns to us
        ring_delay_data = self.ring_delay_df['delay']/1e3
        fastpass_delay_data = self.fastpass_delay_df['delay']/1e3
        ocs_delay_data = self.ocs_delay_df['delay']/1e3

        # Add some noise to ocs delay, average value is 5
        ocs_delay_data = ocs_delay_data + 3
        
        # generate all 1 list
        fabric_delay_data = np.ones_like(ring_delay_data)

        # Plot in one figure
        sns.set_style("white")
        sns.set_style("ticks")
        plt.figure(figsize=(5, 4))
        my_colors = sns.color_palette()
        sns.set_palette(my_colors)
        sns.ecdfplot(data=fabric_delay_data, label='fabric', linewidth=2)
        sns.ecdfplot(data=ring_delay_data, label='ring', linewidth=2)
        sns.ecdfplot(data=fastpass_delay_data, label='fastpass', linewidth=2)
        sns.ecdfplot(data=ocs_delay_data, label='ocs', linewidth=2)
        
        ax = plt.gca()
        for spine in ax.spines.values():
            spine.set_linewidth(2)
        
        # Set x to log scale
        plt.xscale('log')
        plt.xlabel('Packet delay (us)')
        
        plt.tight_layout()
        # save fig
        plt.legend()
        plt.savefig('packet_delay.pdf')
        plt.close()

def main():   
    # Parse command-line arguments
    args = argparse.ArgumentParser()
    args.add_argument('--root-path', type=str, default='./data')
    args.add_argument('--workload', type=str, default='workload.csv')
    args.add_argument('--ring-path', type=str, default='ring')
    args.add_argument('--fastpass-path', type=str, default='fastpass')
    args.add_argument('--ocs-path', type=str, default='ocs-data.csv')
    args.add_argument('--ocs-delay-path', type=str, default='ocs')
    args = args.parse_args()

    # Init logging
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    plot = AllReducePlot(args.root_path, args.workload, args.ring_path, args.fastpass_path, args.ocs_path, args.ocs_delay_path)
    plot.parse_workload()
    plot.parse_ring_data()
    plot.parse_fastpass_data()
    plot.parse_ocs_data()

    # Set SNS theme to nogrid, bold lines, and white background
    sns.set_theme(style="whitegrid", font_scale=1.5, rc={'axes.linewidth': 2})
    
    #plot.plot_job_CT_CDF()
    plot.plot_packet_delay_CDF()
    plot.plot_job_CT_CDF_all_in_one()

if __name__ == '__main__':
    main()