import random
from loguru import logger
from sim_core.network import SyncNetwork, Message
from sim_core.core import Simulator
from sim_core.node import PaxosNode

########################################
#         Run Simulation
########################################
def run_parallel_paxos():
    logger.info("Starting Synchronous Paxos Simulation...")
    sim = Simulator()
    net = SyncNetwork(sim, max_delay=1.9)  # Max delay less than round length
    
    NODE_COUNT = 3
    ROUND_LENGTH = 2.0
    TOTAL_ROUNDS = 10

    # Initialize Nodes
    nodes = []
    for i in range(NODE_COUNT):
        n = PaxosNode(i, NODE_COUNT)
        sim.register_node(n)
        net.register_node(n)
        nodes.append(n)
    
    # Start
    for node in nodes:
        node.start()

    sim.run(max_time=ROUND_LENGTH * TOTAL_ROUNDS)
    
    # Print final logs
    for node in nodes:
        logger.info(f"Final log for {node._id}: {node._log}")

if __name__ == "__main__":
    run_parallel_paxos()
