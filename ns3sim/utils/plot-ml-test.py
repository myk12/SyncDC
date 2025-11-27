import os
import re
import logging
import argparse
import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

class MLPlot:
    def __init__(self, root_path, ocs_msg_path, ring_msg_path, fastpass_msg_path, ocs_reconf_sync_path):
        self.root_path = root_path
        self.ocs_msg_path = ocs_msg_path
        self.ring_msg_path = ring_msg_path  # Placeholder for ring message size path
        self.fastpass_msg_path = fastpass_msg_path  # Placeholder for fastpass message size path
        
        self.ocs_reconf_sync_path = ocs_reconf_sync_path  # Placeholder for OCS reconf sync path
        self.name = os.path.basename(__file__)
    
    def parse_ocs_sync_data(self, sync_dir):
        """
        Parse the OCS sync data from the specified directory.
        """
        logging.info(f"Parsing OCS sync data from {sync_dir}...")
        if not os.path.exists(sync_dir):
            logging.error(f"Sync directory {sync_dir} does not exist.")
            return None
        sync_result = {}
        # List sync dirs
        dir_list = [d for d in os.listdir(sync_dir) if os.path.isdir(os.path.join(sync_dir, d))]
        for d in dir_list:
            # dir format: sync_50ns, current we only support ns
            if not re.match(r'sync_\d+ns', d):
                logging.warning(f"Directory {d} does not match expected format 'sync_50ns'. Skipping.")
                continue
            sync_time_str = d.split('_')[1]  # Extract sync time in ns
            sync_subdir = os.path.join(sync_dir, d)
            
            # Get the result file
            result_file = os.path.join(sync_subdir, 'node-31.log')  # for OCS, the result file is always node-31.log
            if not os.path.exists(result_file):
                logging.error(f"Result file {result_file} does not exist.")
                continue
            # Get the first value of the last line in the result file
            with open(result_file, 'r') as f:
                lines = f.readlines()
                if not lines:
                    logging.warning(f"Result file {result_file} is empty.")
                    continue
                last_line = lines[-1].strip()
                # Extract the first value from the last line which is the result nanoseconds
                first_value = int(last_line.split(',')[0]) if last_line else None
                if first_value is not None:
                    sync_result[sync_time_str] = first_value
                else:
                    logging.warning(f"Could not extract first value from last line in {result_file}.")
        if not sync_result:
            logging.error("No valid OCS sync data found.")
            return None
        # Convert to DataFrame, sync time as index, result as column
        # the key of the dict as index, the value of the dict as column
        df = pd.DataFrame.from_dict(sync_result, orient='index', columns=['result_ns'])
        df.index.name = 'sync_time_ns'  # Set index name
        df.reset_index(inplace=True)  # Reset index to make it a column
        df['sync_time_ns'] = df['sync_time_ns'].astype(str)  # Ensure sync_time_ns is str
        df['result_ns'] = df['result_ns'].astype(int)  # Ensure result_ns is int
        # Sort by the sync time value
        df['sync_time_ns'] = df['sync_time_ns'].str.replace('ns', '').astype(int)
        df = df.sort_values(by='sync_time_ns').reset_index(drop=True)
        logging.info("Parsed OCS sync data successfully.")
        print(df)
        return df
    
    def parse_ocs_reconf_dir_data(self):
        """
        Parse the OCS reconf sync data from the specified path.
        """
        logging.info("Parsing OCS reconf sync data...")
        path = os.path.join(self.root_path, self.ocs_reconf_sync_path)
        if not os.path.exists(path):
            logging.error(f"Path {path} does not exist.")
            return None
        
        df = pd.DataFrame()
        # Get dir list
        dir_list = [d for d in os.listdir(path) if os.path.isdir(os.path.join(path, d))]
        for d in dir_list:
            # dir format: reconf_50us, current we only support us
            if not re.match(r'reconf_\d+us', d):
                logging.warning(f"Directory {d} does not match expected format 'reconf_50us'. Skipping.")
                continue
            reconf_time_str = d.split('_')[1]  # Extract reconf time in
            sync_dir = os.path.join(path, d)

            # Parse the OCS sync data from the reconf directory
            ocs_sync_df = self.parse_ocs_sync_data(sync_dir)
            if ocs_sync_df is not None:
                # Set index to the reconf time
                ocs_sync_df.set_index('sync_time_ns', inplace=True)
                # Rename the result column to include the reconf time
                ocs_sync_df.rename(columns={'result_ns': f'reconf_{reconf_time_str}'}, inplace=True)
                # Ensure the index is a string
                ocs_sync_df.index = ocs_sync_df.index.astype(str)
                # Concatenate the DataFrame to the main DataFrame
                df = pd.concat([df, ocs_sync_df], axis=1)
            
        if df.empty:
            logging.error("No valid OCS reconf sync data found.")
            return None
        return df

    def parse_ocs_msgsize_data(self):
        """
        Parse the OCS message size data from the specified path.
        """
        logging.info("Parsing OCS message size data...")
        path = os.path.join(self.root_path, self.ocs_msg_path)
        if not os.path.exists(path):
            logging.error(f"Path {path} does not exist.")
            return None
        ocs_msgsize_data = {} # msgsize in MB as key, list of values as value
        # Get dir list and parse the message size data and result
        dir_list = [d for d in os.listdir(path) if os.path.isdir(os.path.join(path, d))]
        for d in dir_list:
            # dir format: msgsize_1MB, current we only support MB
            if not re.match(r'msgsize_\d+MB', d):
                logging.warning(f"Directory {d} does not match expected format 'msgsize_1MB'. Skipping.")
                continue
            msg_size_str = d.split('_')[1]  # Extract message size in MB
            # get the result file
            result_file = os.path.join(path, d, 'node-31.log')  # for OCS, the result file is always node-31.log
            if not os.path.exists(result_file):
                logging.error(f"Result file {result_file} does not exist.")
                continue
            # Get the first value of the last line in the result file
            with open(result_file, 'r') as f:
                lines = f.readlines()
                if not lines:
                    logging.warning(f"Result file {result_file} is empty.")
                    continue
                last_line = lines[-1].strip()
                # Extract the first value from the last line which is the result nanoseconds
                first_value = int(last_line.split(',')[0]) if last_line else None # to int
                if first_value is not None:
                    ocs_msgsize_data[msg_size_str] = first_value
                else:
                    logging.warning(f"Could not extract first value from last line in {result_file}.")
        if not ocs_msgsize_data:
            logging.error("No valid OCS message size data found.")
            return None
        return ocs_msgsize_data

    def parse_ring_msgsize_data(self):
        """
        Parse the ring message size data from the specified path.
        """
        logging.info("Parsing ring message size data...")
        path = os.path.join(self.root_path, self.ring_msg_path)
        
        if not os.path.exists(path):
            logging.error(f"Path {path} does not exist.")
            return None
        
        ring_msgsize_data = {} # msgsize in MB as key, list of values as value
        # Get dir list and parse the message size data and result
        dir_list = [d for d in os.listdir(path) if os.path.isdir(os.path.join(path, d))]
        for d in dir_list:
            # dir format: msgsize_1MB, current we only support MB
            if not re.match(r'msgsize_\d+MB', d):
                logging.warning(f"Directory {d} does not match expected format 'msgsize_1MB'. Skipping.")
                continue
            msg_size_str = d.split('_')[1]  # Extract message size in MB
            # get the result file
            # compare the first value of the last line in all the result files,
            # the result file with the largest first value is the result
            result_value = 0
            for f in os.listdir(os.path.join(path, d)):
                with open(os.path.join(path, d, f), 'r') as fs:
                    value = fs.readlines()[-1].split(',')[0]
                    if int(value) > result_value:
                        result_value = int(value)
            ring_msgsize_data[msg_size_str] = result_value
        if not ring_msgsize_data:
            logging.error("No valid ring message size data found.")
            return None
        return ring_msgsize_data

    def plot_jct_barchart(self, output_path="./"):
        """
        Plot a bar chart for the JCT (Job Completion Time) based on the provided DataFrame.
        """
        logging.info("Plotting JCT bar chart...")

        result_data = None
        if self.ocs_msg_path is not None:
            # concatenate the OCS message size data with the result DataFrame
            ocs_data = self.parse_ocs_msgsize_data()
            result_data = pd.DataFrame.from_dict(ocs_data, orient='index', columns=['ocs_jct_ns'])
        
        if self.ring_msg_path is not None:
            # concatenate the ring message size data with the result DataFrame
            ring_data = self.parse_ring_msgsize_data()
            result_data = pd.concat([result_data, pd.DataFrame.from_dict(ring_data, orient='index', columns=['ring_jct_ns'])], axis=1)
        
        if self.fastpass_msg_path is not None:
            # concatenate the fastpass message size data with the result DataFrame
            fastpass_data = self.parse_fastpass_msgsize_data()
        
        # Prepare data
        result_data = result_data.reset_index().rename(columns={'index': 'MessageSize'})
        message_sizes = ['1MB', '2MB', '4MB', '8MB', '16MB']  # Ensure correct order
        result_data['MessageSize'] = pd.Categorical(result_data['MessageSize'], categories=message_sizes, ordered=True)
        result_data = result_data.sort_values('MessageSize')
        
        print(result_data)
        network_types = [col for col in result_data.columns if col != 'MessageSize']
        colors = sns.color_palette("hls", len(network_types))
        hatches = ['x', 'o', '/']  # Different hatches for each network type
        
        # Bar width and positions
        bar_width = 0.8 / len(network_types)  # Adjust width based on number of groups
        x = np.arange(len(message_sizes))
        
        # Plot bars for each network type
        for i, network_type in enumerate(network_types):
            plt.bar(x + (i - len(network_types)/2 + 0.5) * bar_width, 
                    result_data[network_type], 
                    bar_width, 
                    color=colors[i], 
                    edgecolor='black', 
                    hatch=hatches[i % len(hatches)], 
                    label=network_type.replace('_jct_ns', '').capitalize())
            
            # Add value labels
            for j, value in enumerate(result_data[network_type]):
                if not pd.isna(value) and value > 0:
                    plt.text(x[j] + (i - len(network_types)/2 + 0.5) * bar_width, 
                            value, 
                            f'{int(value)}', 
                            ha='center', va='bottom', fontsize=8, color='black')
        
        plt.title('JCT vs Message Size')
        plt.xlabel('Message Size (MB)')
        plt.ylabel('Job Completion Time (ns)')
        plt.yscale('log')  # Use log scale for large range
        plt.xticks(x, message_sizes, rotation=45)
        plt.legend(title='Network Type')
        
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2)
        ax.spines['right'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        
        plt.tight_layout()
        plt.savefig(os.path.join(output_path, 'jct_barchart.pdf'))
        plt.close()
    
    def plot_reconf_sync_linechart(self, output_path="./"):
        """
        Plot a line chart for the OCS reconf sync data.
        """
        logging.info("Plotting OCS reconf sync line chart...")
        
        df = self.parse_ocs_reconf_dir_data()
        if df is None or df.empty:
            logging.error("No data available to plot OCS reconf sync line chart.")
            return
        
        # Plotting
        plt.figure()
        sns.lineplot(data=df, markers=True, markersize=10, linewidth=2.5)
        
        plt.title('OCS JCT vs Reconf time and Sync error')
        plt.xlabel('Clock Sync. Error (ns)')
        plt.ylabel('Job Completion Time (ns)')
        plt.xticks(rotation=45)
        plt.legend(title='Reconf Time (us)', loc='best')
        
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2)
        ax.spines['right'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        
        plt.tight_layout()
        
        # Save the plot
        output_file = os.path.join(output_path, 'reconf_sync_linechart.pdf')
        plt.savefig(output_file)
        plt.close()

def main():
    logging.basicConfig(level=logging.INFO)
    logging.info("Starting MLPlot ...")
    
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Test MLPlot class')
    parser.add_argument('--root_path', type=str, default='result-allreduce', help='Root path for the test files')
    parser.add_argument('--ocs_msg_path', type=str, default='ocs-msgsize', help='Path to the OCS test results file')
    parser.add_argument('--ring_msg_path', type=str, default='ring-msgsize', help='Path to the ring test results file')
    parser.add_argument('--fastpass_msg_path', type=str, default=None, help='Path to the fastpass test results file')
    parser.add_argument('--ocs_reconf_sync_path', type=str, default='ocs-reconf-sync', help='Path to the OCS reconf sync test results file')
    args = parser.parse_args()
    
    ml_ploter = MLPlot(root_path=args.root_path,
                        ocs_msg_path=args.ocs_msg_path,
                        ring_msg_path=args.ring_msg_path,
                        fastpass_msg_path=args.fastpass_msg_path,
                        ocs_reconf_sync_path=args.ocs_reconf_sync_path)
    logging.info(f"MLPlot initialized with root_path: {args.root_path}, ocs_msg_path: {args.ocs_msg_path}, ocs_reconf_sync_path: {args.ocs_reconf_sync_path}")
    ml_ploter.plot_jct_barchart()
    ml_ploter.plot_reconf_sync_linechart()

if __name__ == '__main__':
    print("Running MLPlot test script...")
    main()
