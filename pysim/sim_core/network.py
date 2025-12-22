import random
from dataclasses import dataclass
from typing import Dict
from loguru import logger
from sim_core.core import Simulator

@dataclass
class Message:
    sender: str
    receiver: str
    type: str  # PROPOSE, ACCEPT, DECIDED
    round : int # Paxos round ID
    slot: int  # Log slot ID
    val: str # value being proposed/accepted/decided

# --- 2. Network Simulation Layer ---
class SyncNetwork:
    def __init__(self, simulator: Simulator, max_delay=0.1):
        self.sim = simulator
        self.max_delay = max_delay
        self.nodes = {}

    def register_node(self, node):
        logger.info(f"Network registering node {node._id}")
        self.nodes[node._id] = node
        node.bind_network(self)
    
    def sendto(self, receiver_id: str, msg: Message):
        logger.debug(f"Network sending message {msg.type} from {msg.sender} to {receiver_id} for Slot {msg.slot}")
        if receiver_id in self.nodes:
            delay = random.uniform(0, self.max_delay)
            self.sim.schedule(delay, self.nodes[receiver_id].on_message, msg)
    
    def broadcast(self, sender_node, msg: Message):
        logger.debug(f"Network broadcasting message {msg.type} from {msg.sender} for Slot {msg.slot}")
        for node_id, node in self.nodes.items():
            if node_id != sender_node._id:
                delay = random.uniform(0, self.max_delay)
                self.sim.schedule(delay, node.on_message, msg)
