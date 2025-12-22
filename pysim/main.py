import random
from loguru import logger
from config import load_config
from sim_core.network import SyncNetwork, Message
from sim_core.core import Simulator
from sim_core.node import PaxosNode

########################################
#         Run Simulation
########################################
def main():
    logger.info("Starting Synchronous Paxos Simulation...")
    config = load_config("./sim_conf.toml")

    random.seed(config.simulator.seed)

    sim = Simulator()
    net = SyncNetwork(sim, max_delay=config.system.round_length)  # Max delay less than round length

    # Initialize Nodes
    nodes = []
    for i in range(config.system.num_nodes):
        n = PaxosNode(i, config.system.num_nodes)
        sim.register_node(n)
        net.register_node(n)
        nodes.append(n)
    
    # Start
    for node in nodes:
        node.start()

    sim.run(max_time=config.simulator.max_time)  # Run for 10 rounds
    
    # Print final logs
    for node in nodes:
        logger.info(f"Final log for {node._id}: {node._log}")
        node.save_state()

if __name__ == "__main__":
    main()
