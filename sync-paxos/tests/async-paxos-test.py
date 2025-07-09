import asyncio
import random
import numpy as np
import matplotlib.pyplot as plt
from enum import Enum
from dataclasses import dataclass
from typing import List, Optional
import logging
import seaborn as sns

# Set up minimal logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

completion_times = []

# Node states
class State(Enum):
    FOLLOWER = "FOLLOWER"
    CANDIDATE = "CANDIDATE"
    LEADER = "LEADER"

# Configuration
class Config:
    NUM_NODES = 5
    MESSAGE_LOSS_PROB = 0.01  # 10% message loss
    MEAN_DELAY_US = 1  # Mean delay of 1 microsecond
    NODE_FAILURE_PROB = 0.01  # 5% chance of node failure per run
    ELECTION_TIMEOUT_MS = (10, 20)  # Random timeout range in milliseconds
    CONSENSUS_TIMEOUT_S = 1  # Max time to reach consensus per run
    NUM_RUNS = 100000  # Number of simulation runs
    MAX_SIMULATION_TIME_S = 10  # Safety timeout
    

# Node class
@dataclass
class Node:
    id: int
    state: State = State.FOLLOWER
    term: int = 0
    committed_value: Optional[str] = None
    alive: bool = True

async def run_simulation_once() -> float:
    config = Config()
    sim_time = 0
    node_lists = [Node(id=i) for i in range(config.NUM_NODES)]
    # Randomly fail nodes
    for node in node_lists:
        if random.random() < config.NODE_FAILURE_PROB:
            node.alive = False
    
    # Node0 is the leader
    # Leader crash
    while not node_lists[0].alive:
        logging.info("Node0 is dead, waiting for recovery...")
        # election
        wait_time = random.uniform(*config.ELECTION_TIMEOUT_MS) / 1000
        sim_time += wait_time
        
        # reset nodes
        if random.random() >= config.NODE_FAILURE_PROB:
            node_lists[0].alive = True

    alive_nums = sum(node_lists[i].alive for i in range(config.NUM_NODES))
    # No enough alive nodes
    while alive_nums < (config.NUM_NODES // 2 + 1):
        logging.info(f"Not enough alive nodes to reach consensus, waiting for recovery...")
        wait_time = random.uniform(*config.ELECTION_TIMEOUT_MS) / 1000

        for node in node_lists:
            if random.random() >= config.NODE_FAILURE_PROB:
                node.alive = True
        
        alive_nums = sum(node_lists[i].alive for i in range(config.NUM_NODES))

    # Message Rounds
    consensus = False
    while (not consensus) and (sim_time < config.MAX_SIMULATION_TIME_S): 
        msg_list = []
        for node in node_lists:
            # simluate msg loss
            if random.random() < config.MESSAGE_LOSS_PROB:
                continue
            accept_delay = np.random.lognormal(mean=np.log(1e-6), sigma=1)  # 1µs mean
            accept_ack_delay = np.random.lognormal(mean=np.log(1e-6), sigma=1)  # 1µs mean
            decide_delay = np.random.lognormal(mean=np.log(1e-6), sigma=1)  # 1µs mean
            msg_list.append(sum([accept_delay, accept_ack_delay, decide_delay]))
        if len(msg_list) >= 2:
            consensus = True
            msg_delay = max(msg_list)
            print(msg_delay)
            sim_time += msg_delay
            logging.info(f"Consensus achieved in {sim_time:.6f} seconds")
    return sim_time

async def run_all_simulations(config):
    for _ in range(config.NUM_RUNS):
        time_taken = await run_simulation_once()
        completion_times.append(time_taken)

def plot_cdf():
    # seconds to microseconds
    plot_data = np.array(completion_times) * 1000000
    sns.ecdfplot(plot_data)
    plt.xlabel('Completion Time (us)')
    plt.xscale('log')
    plt.ylabel('Cumulative Probability')
    plt.title('CDF of Consensus Completion Times')
    plt.grid(True)
    plt.legend()
    plt.savefig('async_consensus_cdf.png')
    plt.close()

async def main():
    config = Config()
    await run_all_simulations(config)
    plot_cdf()
    # save data to csv: async-paxos-test.csv
    with open('async-paxos-test.csv', 'w') as f:
        for i in completion_times:
            f.write(f"{i}\n")
    print(f"Simulation complete. CDF plot saved as 'consensus_cdf.png'.")             

if __name__ == "__main__":
    asyncio.run(main())
