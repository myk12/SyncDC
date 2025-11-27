import pandas as pd
import io
import matplotlib.pyplot as plt
import argparse
import os
import seaborn as sns

# Function to parse command-line arguments
def parse_arguments():
    parser = argparse.ArgumentParser(description="Schedule AI inference workload with OCS.")
    parser.add_argument("--transfer-rate", type=float, default=100, help="Network transfer rate in Gbps")
    parser.add_argument("--csv-file", type=str, default="workload.csv", help="Path to CSV file")
    parser.add_argument("--total-servers", type=int, default=32, help="Total number of servers")
    parser.add_argument("--ocs-sync-error", type=int, default=10, help="Time synchronization error in ns")
    parser.add_argument("--ocs-config-time", type=int, default=100, help="Time for changing the circuit in ns")
    args = parser.parse_args()
    if args.transfer_rate <= 0:
        raise ValueError("Transfer rate must be positive")
    return args.transfer_rate, args.csv_file

# Function to load CSV data
def load_csv_file(file_path):
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"CSV file not found at {file_path}")
    with open(file_path, 'r') as f:
        return f.read()

# Function to parse CSV data
def parse_workload(csv_data):
    df = pd.read_csv(io.StringIO(csv_data), sep=',', header=None,
                     names=['job_id', 'servers', 'arrival_time', 'data_volume'],
                     usecols=[0, 1, 2, 3])
    df = df.dropna()
    df = df[df['servers'].isin([4, 6, 8, 10])]
    df['data_volume'] = df['data_volume'].astype(float)  # Data volume in MB
    df['arrival_time'] = df['arrival_time'].astype(float) / 1e9  # Convert ns to seconds
    df['servers'] = df['servers'].astype(int)
    df['job_id'] = df['job_id'].astype(int)
    print(df)
    return df.to_dict('records')

# Function to calculate persist time
def calculate_persist_time(data_volume_mb, num_servers, transfer_rate_gbps):
    # Persist time = (data_volume_MB * 8 / transfer_rate_Gbps) * 10^-3 * num_servers
    persist_time = (data_volume_mb * 8 / transfer_rate_gbps) * 1e-3 * num_servers
    return persist_time

# Scheduling algorithm
def schedule_jobs(jobs, transfer_rate_gbps, total_servers=32):
    print(f"Total servers: {total_servers}")
    if not jobs:
        print("Error: No jobs to schedule")
        return [], 0

    # Calculate persist time for each job
    for job in jobs:
        job['persist_time'] = calculate_persist_time(job['data_volume'], job['servers'], transfer_rate_gbps)
    
    # Sort jobs by arrival time and persist time for SJF
    jobs = sorted(jobs, key=lambda x: (x['arrival_time'], x['persist_time']))
    print(f"Min arrival time: {min(j['arrival_time'] for j in jobs):.6f}s")
    
    schedule = []
    server_timeline = []  # List of (start_time, end_time, servers_used, job_id)
    current_time = 0
    available_servers = total_servers
    pending_jobs = jobs.copy()
    completed_jobs = []
    epsilon = 1e-9  # Small value for floating-point precision

    while pending_jobs or server_timeline:
        print(f"Current time: {current_time:.6f}s, Available servers: {available_servers}")
        # Update available servers by freeing completed jobs
        server_timeline = [(start, end, servers, jid) for start, end, servers, jid in server_timeline if end > current_time + epsilon]
        available_servers = total_servers - sum(servers for _, _, servers, _ in server_timeline)

        # Get jobs that can start
        ready_jobs = [j for j in pending_jobs if j['arrival_time'] <= current_time + epsilon and j['servers'] <= available_servers]
        print(f"Ready jobs: {[j['job_id'] for j in ready_jobs]}")
        if ready_jobs:
            # Select job with shortest persist time
            job = min(ready_jobs, key=lambda x: x['persist_time'])
            start_time = max(current_time, job['arrival_time'])
            end_time = start_time + job['persist_time']
            schedule.append({
                'job_id': job['job_id'],
                'servers': job['servers'],
                'start_time': start_time,
                'end_time': end_time,
                'persist_time': job['persist_time']
            })
            server_timeline.append((start_time, end_time, job['servers'], job['job_id']))
            print(f"Scheduled job {job['job_id']}: Start={start_time:.6f}s, End={end_time:.6f}s, Servers={job['servers']}")
            pending_jobs.remove(job)
            completed_jobs.append(job)
        else:
            # No jobs can start; advance to next event
            print(f"No ready jobs; advance to next event")
            # Only consider jobs with arrival time > current_time
            next_arrival = min([j['arrival_time'] for j in pending_jobs if j['arrival_time'] > current_time + epsilon] + [float('inf')]) if pending_jobs else float('inf')
            next_completion = min([end for _, end, _, _ in server_timeline] + [float('inf')]) if server_timeline else float('inf')
            print(f"Next arrival: {next_arrival:.6f}s, Next completion: {next_completion:.6f}s")
            # Advance to next completion if no servers are available or if it's the next event
            if available_servers == 0 or next_completion <= next_arrival:
                current_time = next_completion
            else:
                current_time = next_arrival
            if current_time == float('inf'):
                print("No more jobs or completions; terminating")
                break

    makespan = max([j['end_time'] for j in schedule]) if schedule else 0
    return schedule, makespan

# Visualization function
def plot_gantt_chart(schedule):
    fig, ax = plt.subplots(figsize=(12, 8))
    colors = {4: 'skyblue', 6: 'lightgreen', 8: 'salmon', 10: 'orchid'}
    for job in schedule:
        ax.barh(job['job_id'], job['end_time'] - job['start_time'], left=job['start_time'],
                height=0.4, color=colors[job['servers']], edgecolor='black',
                label=f"{job['servers']} servers" if job['job_id'] == 1 else "")
    ax.set_xlabel('Time (seconds)')
    ax.set_ylabel('Job ID')
    ax.set_title('Gantt Chart of Job Schedule')
    ax.grid(True)
    ax.legend()
    plt.tight_layout()
    plt.savefig('OCS-GanttChart.pdf')
    plt.close()

# Main execution
def main():
    # Parse command-line arguments
    transfer_rate_gbps, csv_file_path = parse_arguments()
    
    # Load and parse CSV data
    try:
        csv_data = load_csv_file(csv_file_path)
    except FileNotFoundError as e:
        print(e)
        return
    
    jobs = parse_workload(csv_data)
    
    # Schedule jobs
    schedule, makespan = schedule_jobs(jobs, transfer_rate_gbps)
    
    # Output CSV file : job_id, start_time, end_time
    with open('ocs-data.csv', 'w') as f:
        f.write('job_id,start_time,end_time\n')
        for job in schedule:
            # Seconds to nanoseconds and to integer
            job['start_time'] = int(job['start_time']*1e9)
            job['end_time'] = int(job['end_time']*1e9)
            f.write(f"{job['job_id']},{job['start_time']},{job['end_time']}\n")
    
    # Output schedule
    print("Optimized Schedule:")
    for job in sorted(schedule, key=lambda x: x['job_id']):
        print(f"Job {job['job_id']}: Servers={job['servers']}, Start={job['start_time']:.6f}s, "
              f"End={job['end_time']:.6f}s, Persist Time={job['persist_time']:.6f}s")
    print(f"Makespan: {makespan:.6f} seconds")
        
    # Generate Gantt chart
    plot_gantt_chart(schedule)
    
    # join jobs and schedule, adding start and end times
    #jobs = pd.DataFrame(jobs)
    #schedule = pd.DataFrame(schedule)
    #jobs = jobs.merge(schedule, on='job_id', how='left')
    #jobs.to_csv('jobs.csv', index=False)
    
    ## class jobs to different types according to their servers
    ## and then plot the CDF of last time = end_time - arrival_time
    #jobs['type'] = jobs['servers_x'].apply(lambda x: '4' if x == 4 else '6' if x == 6 else '8' if x == 8 else '12')
    #jobs['last_time'] = jobs['end_time'] - jobs['arrival_time']

    ## save to csv
    #jobs.to_csv('jobs.csv', index=False)

    ## for each type, plot the CDF of last time using seaborn
    #sns.set()
    #for type in jobs['type'].unique():
    #    sns.kdeplot(jobs[jobs['type'] == type]['last_time'], cumulative=True, label=type)
    #plt.legend()
    #plt.title('CDF of Last Time')
    #plt.xlabel('Last Time (seconds)')
    #plt.ylabel('CDF')
    #plt.savefig('cdf_last_time.pdf')
    
    # for each type, plot the CDF of persist time using seaborn

if __name__ == "__main__":
    main()
