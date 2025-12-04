import heapq
import itertools
from dataclasses import dataclass, field
from typing import List, Callable
from loguru import logger

################################################
#               Simulation Core
################################################
@dataclass(order=True)
class Event:
    timestamp: float
    uid: int = field(compare=True)    # To ensure FIFO for same timestamp
    # feilds below are not used for comparison
    callback: Callable = field(compare=False)
    name : str = field(compare=False, default="Event")

class Simulator:
    def __init__(self):
        self._clock = 0.0
        self._event_queue: List[Event] = []
        self._nodes = {}
        self._uid_gen = itertools.count()  # Unique ID generator for events
        self._running = False
    
    @property
    def now(self):
        return self._clock
    
    def schedule(self, delay: float, callback: Callable, *args, **kwargs):
        """
        Core function to schedule an event in the simulator.
        :param self: self 
        :param delay: Time delay after which the event should be triggered
        :param callback: Function to be called when the event is triggered
        :param args/kwargs: Positional and keyword arguments to be passed to the callback
        """
        trigger_time = self._clock + delay
        
        # Unique ID for FIFO ordering
        uid = next(self._uid_gen)

        # Warp callback into an Event
        def warpper():
            return callback(*args, **kwargs)
        
        event = Event(timestamp=trigger_time,
                      uid=uid,
                      callback=warpper,
                      name=callback.__name__)

        heapq.heappush(self._event_queue, event)
    
    def run(self, max_time: float = float('inf')):
        self._running = True
        while self._event_queue and self._running:
            # Peek at the next event
            event = heapq.heappop(self._event_queue)

            # Stop if we exceed max_time
            if event.timestamp > max_time:
                break

            # Advance clock
            self._clock = event.timestamp
            logger.debug(f"Time {self._clock:.3f}: Executing event {event.name}")

            # Execute the event's callback
            event.callback()
        
        logger.info("Simulation ended at time {:.3f}".format(self._clock))

    def register_node(self, node):
        logger.info(f"Registering node {node._id}")
        self._nodes[node._id] = node
        node.bind_simulator(self)
