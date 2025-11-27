import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import re
import matplotlib.ticker as mticker
from matplotlib.ticker import FuncFormatter
import math
import xml.etree.ElementTree as ET
import logging  
import argparse
import numpy as np
import yaml

def format_large_numbers(value, _):
    if value >= 1_000_000_000:
        return f"{int(value // 1_000_000_000)}G"
    elif value >= 1_000_000:
        return f"{int(value // 1_000_000)}M"
    elif value >= 1_000:
        return f"{int(value // 1_000)}k"
    else:
        return f"{int(value)}"

def format_percentage(y, pos):
    """Format y-axis ticks as percentages."""
    return f'{y:.0f}%'

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
    def __init__(self, results_root_dir, simulation_seconds, flow_monitor_xml, system_topology_yaml):
        self.results_root_dir = results_root_dir
        self.simulation_seconds = simulation_seconds
        
        print(f"Results root directory: {results_root_dir}")
        print(f"Simulation duration: {simulation_seconds} seconds for Opps calculation.")
        
        self.sync_res_dir = os.path.join(results_root_dir, "Sync")
        self.async_res_dir = os.path.join(results_root_dir, "Async")

        self.flow_monitor_xml = flow_monitor_xml
        self.system_topology_yaml = system_topology_yaml

    def plot_sync_opps(self):
        # Plot the results to a line chart
        # X-axis: delay bound
        # Y-axis: number of operations per second
        plt.figure()
        plt.yscale('log')
        sns.set_style('whitegrid')
        sns.color_palette("hls", len(self.sync_result_df.columns))
        sns.lineplot(data=self.sync_result_df, markers=True, markersize=10, linewidth=2.5, dashes=False)

        plt.xlabel("Clock Sync. Error", fontsize=14)
        plt.ylabel("Operations per second", fontsize=14)
        plt.xticks(rotation=45, ha='right', fontsize=12)
        plt.yticks(fontsize=12)
        plt.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5)
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2) 
        ax.spines['right'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        # make the legend outside the plot
        # https://stackoverflow.com/questions/4700614/how-to-put-the-legend-out-of-the-plot
        plt.legend(
            title='Delay Bound',
            bbox_to_anchor=(0.5, 1.1),  # Position outside, above the plot
            loc='center',
            bbox_transform=ax.transAxes,
            ncol=5,  # Spread items in one row
            borderaxespad=0.0  # Minimize padding between legend and plot
        )
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
        sns.barplot(data=self.async_result_df)
        
        plt.xlabel("One way delay", fontsize=14)
        plt.ylabel("Operations per second (log scale)", fontsize=14)
        plt.xticks(rotation=45, ha='right', fontsize=12)
        plt.yticks(fontsize=12)
        #plt.grid(True, color='gray', linestyle='--', linewidth=0.5)
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2) 
        ax.spines['right'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        plt.tick_params(axis='both', colors='black')
        plt.gca().yaxis.set_major_formatter(FuncFormatter(format_large_numbers))

        # Plot the sync result points as comparison
        sync_results = self.sync_result_df.loc["50ns"]
        # generate different colors and markers for each sync result point
        colors = sns.color_palette("hls", len(sync_results))
        markers = ['o', 's', '^', 'D', 'v']
        points = []
        # Get x-tick positions from the barplot
        x_positions = range(len(self.async_result_df.columns))  # Assuming async_result_df columns match x-ticks
        for i, (sync_res, col_name) in enumerate(zip(sync_results, self.sync_result_df.columns)):
            # Find matching x-position based on column name
            try:
                x_idx = list(self.async_result_df.columns).index(col_name)
                point = plt.scatter(x_idx, sync_res, color=colors[i], marker=markers[i], s=100, 
                                label=col_name, edgecolors='black', linewidth=1.5)
                points.append(point)
            except ValueError:
                print(f"Warning: Column {col_name} not found in async_result_df columns")
        
        plt.legend(handles=points, title="Delay Bound")
        
        print(sync_results)

        #plt.tight_layout()
        pdf_path = os.path.join(self.results_root_dir, "async-opps.pdf")
        plt.savefig(pdf_path, dpi=300, format='pdf', bbox_inches='tight')
        plt.close()
        

    def parse_sync_result_dir(self):
        # Parse the sync test results
        print(f"Reading sync test results from '{self.sync_res_dir}' directory...")        
        results_dict = {}
        packets_dict = {}
        # list dirs in this directory
        sync_dirs = os.listdir(self.sync_res_dir)
        for delay_dir in sync_dirs:
            # Parse the delay bound
            # Dir name format: Delay_50us
            delay_bound = int(re.search(r"Delay_(\d+)us", delay_dir).group(1))
            results_dict[f"{delay_bound}us"] = {}
            packets_dict[f"{delay_bound}us"] = {}

            for sync_dir in os.listdir(os.path.join(self.sync_res_dir, delay_dir)):
                # Parse the sync error. Dir name format: Sync_100ns
                sync_err = int(re.search(r"Sync_(\d+)ns", sync_dir).group(1))
                current_dir = os.path.join(self.sync_res_dir, delay_dir, sync_dir)
                print(sync_err, current_dir)
                
                # Parse the number of operations. Get the number of lines of the first file in the directory)
                file = os.listdir(current_dir)[0]
                # get file lines
                num_lines = len(open(os.path.join(current_dir, file), 'r').readlines())

                # Calculate the number of operations per second
                opps = num_lines / self.simulation_seconds
                results_dict[f"{delay_bound}us"][f"{sync_err}ns"] = opps
                
                # Parse the packets number of each server
                server_packets = {}
                for file in os.listdir(current_dir):
                    # parse server ID from filename: server-0-decision.log
                    server_id = int(file.split('-')[1])
                    # parse packets number from file: last two lines
                    lines = open(os.path.join(current_dir, file), 'r').readlines()
                    send_packets = int(lines[-2].split(' ')[-1])
                    recv_packets = int(lines[-1].split(' ')[-1])
                    server_packets[server_id] = (send_packets, recv_packets)
                packets_dict[f"{delay_bound}us"][f"{sync_err}ns"] = server_packets
        
        # Transform the dictionary to a Pandas DataFrame
        self.sync_result_df = pd.DataFrame.from_dict(results_dict).sort_index(axis=0).sort_index(axis=1)
        print(f"Sync test results:\n{self.sync_result_df}")
        print(self.sync_result_df.describe())

        self.sync_packets_dict = packets_dict
        print(self.sync_packets_dict)
    
    def parse_async_result_dir(self):
        # Parse the async test results
        print(f"Reading async test results from '{self.async_res_dir}' directory...")        
        results_dict = {}
        packets_dict = {}
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
            
            packets_dict[f"{e2e_delay}us"] = {}

            for file in os.listdir(current_dir):
                # parse server ID from filename: server-0-decision.log
                server_id = int(file.split('-')[1])
                # parse packets number from file: last two lines
                lines = open(os.path.join(current_dir, file), 'r').readlines()
                send_packets = int(lines[-2].split(' ')[-1])
                recv_packets = int(lines[-1].split(' ')[-1])

                packets_dict[f"{e2e_delay}us"][server_id] = (send_packets, recv_packets)
                

            results_dict[f"{e2e_delay}us"] = opps
        
        print(f"Async test results:\n{results_dict}")
            
        # Transform the dictionary to a Pandas DataFrame
        self.async_result_df = pd.DataFrame(results_dict, index=[0]).sort_index(axis=1)
        print(f"Async test results:\n{self.async_result_df}")
        
        self.async_packets_dict = packets_dict
        print(self.async_packets_dict)
    
    def parse_sync_flow_monitor_XML(self):
        # Parse the XML file
        sync_flow_monitor_xml = "sync-"+os.path.basename(self.flow_monitor_xml)
        
        self.sync_node_packets = self.parse_flow_monitor_XML(sync_flow_monitor_xml)
    
    def parse_async_flow_monitor_XML(self):
        # Parse the XML file
        async_flow_monitor_xml = "async-"+os.path.basename(self.flow_monitor_xml)
        
        self.async_node_packets = self.parse_flow_monitor_XML(async_flow_monitor_xml)
        
    
    def parse_flow_monitor_XML(self, flow_monitor_xml):
        # Parse the XML file
        print(f"Reading flow monitor XML file: {flow_monitor_xml}")
        tree = ET.parse(flow_monitor_xml)
        root = tree.getroot()
        
        # List to store flow data
        flow_data = []
        flow_packets = {}
        # Find all Flow elements in FlowStats section
        flow_stats = root.find(".//FlowStats")
        # Iterate over each Flow element
        for flow in flow_stats.findall("Flow"):
            flow_id = flow.get("flowId")
            tx_packets = flow.get("txPackets")
            rx_packets = flow.get("rxPackets")
            
            # Convert to integers
            flow_data.append({"flowId": int(flow_id), "txPackets": int(tx_packets), "rxPackets": int(rx_packets)})
            flow_packets[flow_id] = (int(tx_packets), int(rx_packets))
        
        # Endpoint map to flows
        endpoint_packets = {}
        flow_ends = root.find(".//Ipv4FlowClassifier")
        for flow in flow_ends.findall("Flow"):
            flow_id = flow.get("flowId")
            src_node = flow.get("sourceAddress")
            dst_node = flow.get("destinationAddress")

            # Add to the endpoint map
            if src_node not in endpoint_packets:
                endpoint_packets[src_node] = flow_packets[flow_id][0]
            else:
                endpoint_packets[src_node] += flow_packets[flow_id][0]
            
            if dst_node not in endpoint_packets:
                endpoint_packets[dst_node] = flow_packets[flow_id][1]
            else:
                endpoint_packets[dst_node] += flow_packets[flow_id][1]

        return endpoint_packets

    def parse_system_topology_yaml(self):
        # Parse the system topology YAML file
        print(f"Reading system topology YAML file: {self.system_topology_yaml}")
        self.server_ids = None
        with open(self.system_topology_yaml, 'r') as f:
            data = yaml.safe_load(f)

            # Create mapping of id to ip_address
            self.server_ids = {
                server['id']: server['ip_address']
                for server in data['servers']
            }
        
        print(self.server_ids)    
    
    def plot_rack_traffic_volume(self):
        sync_traffic = {}
        for server_id, ip_address in self.server_ids.items():
            sync_traffic[server_id] = self.sync_node_packets[ip_address]
        
        async_traffic = {}
        for server_id, ip_address in self.server_ids.items():
            async_traffic[server_id] = self.async_node_packets[ip_address]

        print(sync_traffic)
        print(async_traffic)
        
        # Plot the group bar chart
        # Extract rack IDs (assume same for both datasets)
        rack_ids = sorted(sync_traffic.keys())
        
        # Compute totals for normalization
        sync_total = sum(sync_traffic.values())
        async_total = sum(async_traffic.values())
        
        # Normalize traffic to percentages
        sync_percent = [sync_traffic[rack_id] / sync_total * 100 for rack_id in rack_ids]
        async_percent = [async_traffic[rack_id] / async_total * 100 for rack_id in rack_ids]
        
        # Set up bar positions
        bar_width = 0.35
        x = np.arange(len(rack_ids))
        
        # Set up bar positions
        bar_width = 0.35
        x = np.arange(len(rack_ids))
        
        # Create figure and axis
        plt.figure(figsize=(8, 6))
        
        colors = sns.color_palette("hls", 2)
        
        # Plot grouped bars with consistent style
        plt.bar(x - bar_width/2, sync_percent, bar_width, 
                color=colors[0], edgecolor='black', hatch='x', label='Synchronous')
        plt.bar(x + bar_width/2, async_percent, bar_width, 
                color=colors[1], edgecolor='black', hatch='o', label='Asynchronous')
        
        # Customize plot to match plot_async_opps style
        plt.xlabel("Rack ID", fontsize=14)
        plt.ylabel("Traffic Percentage", fontsize=14)
        plt.xticks(x, rack_ids, fontsize=12)
        plt.yticks(fontsize=12)

        plt.gca().yaxis.set_major_formatter(FuncFormatter(format_percentage))
        
        # Set spine linewidths
        ax = plt.gca()
        ax.spines['top'].set_linewidth(2)
        ax.spines['right'].set_linewidth(2)
        ax.spines['bottom'].set_linewidth(2)
        ax.spines['left'].set_linewidth(2)
        
        # Set tick colors
        plt.tick_params(axis='both', colors='black')
        
        # Add legend
        plt.legend(loc='best', fontsize=12, title="System Type")

        # Save and show
        plt.tight_layout()
        pdf_path = os.path.join(self.results_root_dir, "rack-traffic-volume.pdf")
        plt.savefig(pdf_path, dpi=300, format='pdf', bbox_inches='tight')
    
    def plot_opps_comparison(self):
        # Plot the results to a group bar chart
        # X-axis: delay bound
        # Y-axis: number of operations per second
        # subplot 1x2

        # Melt DataFrames
        sync_melted = self.sync_result_df.reset_index().melt(id_vars='index', var_name='delay bound', value_name='operation per second')
        sync_melted.rename(columns={'index': 'clock_sync_error'}, inplace=True)
        sync_melted['type'] = 'Sync'

        async_melted = self.async_result_df.reset_index().melt(id_vars='index', var_name='one way delay', value_name='operation per second')
        async_melted.rename(columns={'index': 'clock_sync_error'}, inplace=True)
        async_melted['clock_sync_error'] = 'Async'  # Assign single label for async
        async_melted['type'] = 'Async'

        # Combine DataFrames
        df_combined = pd.concat([sync_melted, async_melted], ignore_index=True)

        # Set Seaborn style
        sns.set_style("whitegrid")

        # Create figure with two subplots, sharing y-axis
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(8, 3), sharey=True, gridspec_kw={'width_ratios':[0.73,0.27]})

        # Plot Sync bar chart (grouped)
        sns.set_palette("hls", 3)
        sns.barplot(
            data=df_combined[df_combined['type'] == 'Sync'],
            x='delay bound',
            y='operation per second',
            hue='clock_sync_error',
            ax=ax1
        )

        ax1.set_title('Sync. Paxos')
        ax1.set_xlabel('Delay Bound')
        ax1.set_ylabel('Operations per second')
        ax1.set_yscale('log')
        ax1.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5)
        ax1.legend(title='Clock Sync Error')

        # Plot Async bar chart (single bars)
        sns.barplot(
            data=df_combined[df_combined['type'] == 'Async'],
            x='one way delay',
            y='operation per second',
            hue='clock_sync_error',
            ax=ax2
        )
        ax2.set_title('Async. Paxos')
        ax2.set_xlabel('One Way Delay')
        ax2.set_ylabel('')
        ax2.grid(True, which='both', axis='both', linestyle='--', linewidth=0.5)
        ax2.get_legend().remove()

        # Adjust layout
        fig.subplots_adjust(wspace=0.01, top=0.85)
        plt.savefig(os.path.join(self.results_root_dir, "opps-comparison.pdf"), dpi=300, format='pdf', bbox_inches='tight')

    def plot_traffic_volume(self):
        # Calculate sync packets
        sync_server_volume = {}
        for delay_bound, sync_err_dict in self.sync_packets_dict.items():
            for sync_err, server_packets in sync_err_dict.items():
                num_ops = self.sync_result_df.loc[sync_err][delay_bound]
                server_MBkops = {}
                for server_id, packets in server_packets.items():
                    server_MBkops[server_id] = sum(packets) * 512 / num_ops * 1000 / 1024 / 1024 # Bytes/kops
                print(server_MBkops)
        
        async_servers_volume = {}
        for delay_bound, server_packets in self.async_packets_dict.items():
            for server_id, packets in server_packets.items():
                num_ops = self.async_result_df.loc[0][delay_bound]
                server_MBkops = {}
                for server_id, packets in server_packets.items():
                    server_MBkops[server_id] = sum(packets) * 512 / num_ops * 1000 / 1024 / 1024 # Bytes/kops
                print(server_MBkops)
    

if __name__ == "__main__":
    # Set up logging
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    logger = logging.getLogger(__name__)

    # Parse the command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--results-dir", type=str, default="result", help="Base directory for the results.")
    parser.add_argument("--runtime", type=int, default=5, help="Simulation runtime in seconds.")
    parser.add_argument("--flow-monitor-xml", type=str, default="flow-monitor.xml", help="Path to the flow monitor XML file.")
    parser.add_argument("--system-topology", type=str, default="system-topology.yaml", help="Path to the system topology YAML file.")
    args = parser.parse_args()
    
    # Create a PaxosPlot object
    plot = PaxosPlot(args.results_dir, args.runtime, args.flow_monitor_xml, args.system_topology)
    plot.parse_sync_result_dir()
    plot.parse_async_result_dir()
    #plot.parse_sync_flow_monitor_XML()
    #plot.parse_async_flow_monitor_XML()
    #plot.parse_system_topology_yaml()
    #plot.plot_sync_opps()
    #plot.plot_async_opps()
    #plot.plot_rack_traffic_volume()
    plot.plot_opps_comparison()
    plot.plot_traffic_volume()