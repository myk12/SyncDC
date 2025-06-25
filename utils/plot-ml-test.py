import os
import re
import logging
import argparse
import sys
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

class MLPlot:
    def __init__(self, root_path, ocs_msg_path, ocs_reconf_sync_path):
        self.root_path = root_path
        self.ocs_msg_path = ocs_msg_path
        self.ring_msg_path = None  # Placeholder for ring message size path
        self.fastpass_msg_path = None
        
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
        # Convert to DataFrame
        df = pd.DataFrame(list(ocs_msgsize_data.items()), columns=['msg_size_MB', 'result_ns'])
        df['msg_size_MB'] = df['msg_size_MB'].astype(str)  # Ensure msg_size_MB is str
        df['result_ns'] = df['result_ns'].astype(int)  # Ensure result_ns is int
        # Sort by the message size value 
        df['msg_size_MB'] = df['msg_size_MB'].str.replace('MB', '').astype(int)
        df = df.sort_values(by='msg_size_MB').reset_index(drop=True)
        logging.info("Parsed OCS message size data successfully.")
        return df

    def plot_jct_barchart(self, output_path="./"):
        """
        Plot a bar chart for the JCT (Job Completion Time) based on the provided DataFrame.
        """
        logging.info("Plotting JCT bar chart...")

        result_df = None
        if self.ocs_msg_path is not None:
            # concatenate the OCS message size data with the result DataFrame
            ocs_df = self.parse_ocs_msgsize_data()
            result_df = pd.concat([result_df, ocs_df], ignore_index=True)
        
        if self.ring_msg_path is not None:
            # concatenate the ring message size data with the result DataFrame
            ring_df = self.parse_ring_msgsize_data()
            result_df = pd.concat([result_df, ring_df], ignore_index=True)
        
        if self.fastpass_msg_path is not None:
            # concatenate the fastpass message size data with the result DataFrame
            fastpass_df = self.parse_fastpass_msgsize_data()
            result_df = pd.concat([result_df, fastpass_df], ignore_index=True)
        
        if result_df is None or result_df.empty:
            logging.error("No data available to plot JCT bar chart.")
            return
        # Plotting
        plt.figure(figsize=(8, 6))
        sns.barplot(x='msg_size_MB', y='result_ns', data=result_df)
        
        plt.title('Job Completion Time (JCT) vs Message Size')
        plt.xlabel('Message Size (MB)')
        plt.ylabel('Job Completion Time (ns)')
        plt.xticks(rotation=45)
        plt.tight_layout()
        
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2)
        ax.spines['right'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)

        # Save the plot
        output_file = os.path.join(output_path, 'jct_barchart.pdf')
        plt.savefig(output_file)
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
    parser.add_argument('--ocs_reconf_sync_path', type=str, default='ocs-reconf-sync', help='Path to the OCS reconf sync test results file')
    args = parser.parse_args()
    
    ml_ploter = MLPlot(root_path=args.root_path,
                        ocs_msg_path=args.ocs_msg_path,
                        ocs_reconf_sync_path=args.ocs_reconf_sync_path)
    logging.info(f"MLPlot initialized with root_path: {args.root_path}, ocs_msg_path: {args.ocs_msg_path}, ocs_reconf_sync_path: {args.ocs_reconf_sync_path}")
    ml_ploter.plot_jct_barchart()
    ml_ploter.plot_reconf_sync_linechart()

if __name__ == '__main__':
    print("Running MLPlot test script...")
    main()
