import numpy as np
import pandas as pd
import random
import argparse

# Parse command-line arguments
def parse_args():
    parser = argparse.ArgumentParser(description="Generate AI inference workload with different traffic modes for NS-3 simulation.")
    parser.add_argument("--num_jobs", type=int, default=1000, help="Number of jobs to generate (default: 1000)")
    parser.add_argument("--mean_inter_arrival_time", type=float, default=2.0, 
                        help="Mean inter-arrival time for Poisson distribution (ms, default: 2.0)")
    parser.add_argument("--request_nodes", type=str, default="4,8,16,32", 
                        help="Comma-separated list of possible node counts (default: 4,8,16,32)")
    parser.add_argument("--data_size", type=str, default="1,2", 
                        help="Comma-separated list of possible data sizes in MB (default: 1,2)")
    parser.add_argument("--output_file", type=str, default="ai_inference_workload.csv", 
                        help="Output CSV file name (default: ai_inference_workload.csv)")
    parser.add_argument("--traffic_mode", type=str, default="poisson", 
                        choices=["poisson", "bursty", "periodic"], 
                        help="Traffic mode: poisson, bursty, or periodic (default: poisson)")
    parser.add_argument("--burst_interval", type=float, default=60.0, 
                        help="Interval between bursts in seconds (default: 60.0)")
    parser.add_argument("--burst_duration", type=float, default=5.0, 
                        help="Duration of each burst in seconds (default: 5.0)")
    parser.add_argument("--burst_lambda", type=float, default=0.5, 
                        help="Mean inter-arrival time during bursts (ms, default: 0.5)")
    parser.add_argument("--periodic_interval", type=float, default=2.0, 
                        help="Interval for periodic traffic (ms, default: 2.0)")
    return parser.parse_args()

# Generate create_time based on specified traffic mode
def generate_create_times(num_jobs, traffic_mode, mean_inter_arrival_time, 
                         burst_interval, burst_duration, burst_lambda, periodic_interval):
    # Convert time parameters to nanoseconds
    mean_inter_arrival_time_ns = mean_inter_arrival_time * 1000000
    burst_interval_ns = burst_interval * 1000000000
    burst_duration_ns = burst_duration * 1000000000
    burst_lambda_ns = burst_lambda * 1000000
    periodic_interval_ns = periodic_interval * 1000000

    create_times = []
    current_time = 0

    if traffic_mode == "poisson":
        # Pure Poisson distribution
        intervals = np.random.poisson(mean_inter_arrival_time_ns, num_jobs)
        create_times = intervals.cumsum()

    elif traffic_mode == "bursty":
        # Bursty traffic: alternate between normal and burst periods
        for _ in range(num_jobs):
            # Check if in burst period
            if (current_time % burst_interval_ns) < burst_duration_ns:
                # Burst period: high request rate (small interval)
                interval = np.random.poisson(burst_lambda_ns)
            else:
                # Normal period: normal request rate
                interval = np.random.poisson(mean_inter_arrival_time_ns)
            current_time += interval
            create_times.append(current_time)

    elif traffic_mode == "periodic":
        # Periodic traffic: fixed interval with small Poisson noise
        noise = np.random.normal(0, mean_inter_arrival_time_ns * 0.1, num_jobs)  # 10% noise
        intervals = np.full(num_jobs, periodic_interval_ns) + noise
        create_times = intervals.cumsum().astype(int)

    return np.array(create_times)

def main():
    # Parse command-line arguments
    args = parse_args()

    # Process parameters
    num_jobs = args.num_jobs
    mean_inter_arrival_time = args.mean_inter_arrival_time
    request_nodes_options = [int(x) for x in args.request_nodes.split(",")]
    data_size_options = [int(x) for x in args.data_size.split(",")]
    output_file = args.output_file
    traffic_mode = args.traffic_mode
    burst_interval = args.burst_interval
    burst_duration = args.burst_duration
    burst_lambda = args.burst_lambda
    periodic_interval = args.periodic_interval

    # Set random seed for reproducibility
    np.random.seed(1234)
    random.seed(1234)

    # Generate create_time
    create_times = generate_create_times(
        num_jobs, traffic_mode, mean_inter_arrival_time, 
        burst_interval, burst_duration, burst_lambda, periodic_interval
    )

    # Generate workload data
    data = {
        'jobid': range(1, num_jobs + 1),
        'request_nodes': [random.choice(request_nodes_options) for _ in range(num_jobs)],
        'create_time': create_times,
        'data_size': [random.choice(data_size_options) for _ in range(num_jobs)],
        'start_time': [''] * num_jobs,  # Empty start_time column
        'end_time': [''] * num_jobs     # Empty end_time column
    }

    # Create DataFrame
    df = pd.DataFrame(data)

    # Save to CSV file
    df.to_csv(output_file, index=False)

    # Print information
    print(f"Workload generated with {num_jobs} jobs and saved to {output_file}")
    print("Parameters used:")
    print(f"  Traffic mode: {traffic_mode}")
    print(f"  Mean inter-arrival time: {mean_inter_arrival_time} ms")
    print(f"  Request nodes options: {request_nodes_options}")
    print(f"  Data size options: {data_size_options} MB")
    if traffic_mode == "bursty":
        print(f"  Burst interval: {burst_interval} s, Burst duration: {burst_duration} s, Burst lambda: {burst_lambda} ms")
    elif traffic_mode == "periodic":
        print(f"  Periodic interval: {periodic_interval} ms")
    print("\nFirst few rows of the workload:")
    print(df.head())

    # Visualize create_time distribution
    import matplotlib.pyplot as plt
    plt.hist(df['create_time'] / 1000000, bins=50, density=True)  # Convert to ms
    plt.title(f'Create Time Distribution (ms, {traffic_mode} mode)')
    plt.xlabel('Create Time (ms)')
    plt.ylabel('Density')
    plt.savefig(f"create_time_distribution_{traffic_mode}.png")
    plt.close()
    print(f"Create time distribution plot saved as create_time_distribution_{traffic_mode}.png")

if __name__ == "__main__":
    main()
