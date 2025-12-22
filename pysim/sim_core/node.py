import os
from typing import Dict
from loguru import logger
from sim_core.network import SyncNetwork, Message
from sim_core.core import Simulator 
# For each round n, the node does 3 things:
# 1. As Proposer, determine which Slot it is responsible for round n and start Paxos for that Slot
# 2. As Acceptor, respond to incoming PREPARE for round n - 1
# 3. As Acceptor, respond to incoming ACCEPT for round n - 2

class PaxosNode:
    def __init__(self, node_id: int, num_nodes: int):
        self._id : str = f"Node-{node_id}"
        self._idx: int = node_id
        self._num_nodes = num_nodes

        # Storage for decided values: Slot ID -> Value
        self._log: Dict[int, str] = {}
        
        # Acceptor 状态存储: Slot ID -> {min_p, acc_p, acc_v}
        self._acceptor_states = {}
        
        # Proposer 状态存储: Slot ID -> {promises: [], accepted_count: 0, state: '...'}
        self._proposer_states = {}
        
        self._quorum = (num_nodes // 2) + 1
    
    def bind_network(self, network: SyncNetwork):
        self._network = network
    
    def bind_simulator(self, simulator: Simulator):
        self._simulator = simulator

    def _start_round_event(self, round_id):
        self.on_start_round(round_id)
        
        # Schedule next round start
        ROUND_LENGTH = 2.0
        self._simulator.schedule(ROUND_LENGTH, self._start_round_event, round_id + 1)
    
    def start(self):
        logger.info(f"{self._id} starting...")
        self._start_round_event(0)

    def log_event(self, msg: str):
        logger.info(f"[{self._id} @ {self._simulator.now:.3f}] {msg}")

    def get_my_slot(self, round_id):
        # Predetermine which slot this node is responsible for in this round
        # Round 0: Node0->Slot0, Node1->Slot1, Node2->Slot2
        # Round 1: Node0->Slot3, Node1->Slot4, Node2->Slot5
        return round_id * self._num_nodes + self._idx

    def on_start_round(self, round_id: int):
        # 1. Calculate which Slot I am responsible for this round
        my_slot = self.get_my_slot(round_id)
        self.log_event(f"Starting Round {round_id}, responsible for Slot {my_slot}")
        
        # 2. Initialize Proposer state for this Slot
        propose = {
            'round': round_id,
            'value' : f'Value_from_{self._id}_Slot_{my_slot}',
            'promises': [], 
            'accepted_count': 0,
            'finished': False,
        }
        self._proposer_states[my_slot] = propose

        # 3. Start Paxos for my Slot by sending PROPOSE
        msg = Message(self._id, "", 'PROPOSE', round_id, my_slot, val=propose['value'])
        self._network.broadcast(self, msg)

    def on_message(self, msg: Message):
        # check if the message is for me
        if msg.receiver and msg.receiver != self._id:
            self.log_event(f"Ignoring message {msg.type} intended for {msg.receiver}")
            return

        self.log_event(f"Received message {msg.type} from {msg.sender} for Slot {msg.slot}")
        if msg.type == 'PROPOSE':
            self.handle_propose(msg)
        elif msg.type == 'ACCEPT':
            self.handle_accept(msg)
        elif msg.type == 'DECIDED':
            self.handle_decided(msg)

    # --- handler for Acceptor logic (Phase 1a and 2a) ---
    def handle_propose(self, msg: Message):
        # extract acceptor state for the slot
        self.log_event(f"Handling PROPOSE for Slot {msg.slot} from {msg.sender} in Round {msg.round}")
        self._acceptor_states[msg.slot] = {
            "proposer_id": msg.sender,
            "round": msg.round,
            "accepted_value": msg.val,
            "accepted_round": msg.round,
            "decided": False
        }

        # send back ACKNOWLEDGE
        ack_msg = Message(
            sender=self._id,
            receiver=msg.sender,
            type='ACCEPT',
            round=msg.round,
            slot=msg.slot,
            val=msg.val
        )
        self._network.sendto(msg.sender, ack_msg)
            
    # --- handler for acceptor logic (Phase 1b and 2b) ---
    def handle_accept(self, msg: Message):
        self.log_event(f"Handling ACCEPT for Slot {msg.slot} from {msg.sender} in Round {msg.round}")
        
        try: 
            proposer_state = self._proposer_states[msg.slot]
        except KeyError:
            self.log_event(f"No proposer state found for Slot {msg.slot}, ignoring ACCEPT")
            return

        # Record the promise
        proposer_state['promises'].append(msg.sender)
        proposer_state['accepted_count'] += 1
        
        if proposer_state['accepted_count'] >= self._quorum and not proposer_state['finished']:
            logger.info(f"Proposer {self._id} for Slot {msg.slot} has achieved quorum with {proposer_state['accepted_count']} ACCEPTED messages.")
            # achieve quorum, send ACCEPTED
            proposer_state['finished'] = True
            self.log_event(f"*** Slot {msg.slot} achieved consensus! ***")
            # Broadcast DECIDED
            dec = Message(self._id, "", 'DECIDED', msg.round, msg.slot, val=proposer_state['value'])
            self._network.broadcast(self, dec)
            
            # Record decided value in own log
            self._log[msg.slot] = proposer_state['value']

    # --- handler for decided messages ---
    def handle_decided(self, msg: Message):
        self.log_event(f"Handling DECIDED for Slot {msg.slot} from {msg.sender} in Round {msg.round}")
        # Record the decided value in the log
        if msg.slot not in self._acceptor_states:
            # error case, but just log it
            self.log_event(f"Warning: DECIDED for unknown Slot {msg.slot}")

        self._acceptor_states[msg.slot]['decided'] = True
        self._log[msg.slot] = msg.val
        self.log_event(f"Slot {msg.slot} decided with value: {msg.val}")
    
    ###############################################
    # Additional helper methods can be added here #
    ###############################################
    def save_state(self, output_dir : str = "./results/"):
        os.makedirs(output_dir, exist_ok=True)
        filepath = os.path.join(output_dir, f"{self._id}_log.txt")

        with open(filepath, 'w') as f:
            for slot in sorted(self._log.keys()):
                f.write(f"Slot {slot}: {self._log[slot]}\n")
        self.log_event(f"State saved to {filepath}")
 