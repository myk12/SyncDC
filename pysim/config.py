import sys
import tomllib

from typing import Literal, Optional
from pydantic import BaseModel, Field, model_validator, ValidationError
from pathlib import Path

from loguru import logger

class SimulatorConfig(BaseModel):
    seed: int = 42
    max_time : int = Field(1000, description="Maximum simulation time in nanoseconds")
    time_step: int = Field(1, description="Time step for the simulation in nanoseconds")
    output_dir: str = "./results"

class NetworkConfig(BaseModel):
    delay_bound : int = Field(490, description="Maximum network delay in nanoseconds")
    
    # delay distribution can be 'uniform', 'normal', or 'exponential'
    delay_distribution: Literal['uniform', 'normal', 'exponential'] = 'exponential'
    packet_loss_rate: float = Field(0.01, description="Packet loss rate as a float between 0 and 1")

class NodeConfig(BaseModel):
    exec_bound: int = Field(490, description="Maximum execution time for a node in nanoseconds")
    
    # failure distribution can be 'uniform', 'normal', or 'exponential'
    failure_distribution: Literal['uniform', 'normal', 'exponential'] = 'exponential'
    failure_rate: float = Field(0.01, description="Node failure rate as a float between 0 and 1")

class SystemConfig(BaseModel):
    num_nodes: int = Field(5, description="Number of nodes in the distributed system")
    round_length: Optional[int] = Field(None, description="Length of each round in nanoseconds")
    
class Config(BaseModel):
    simulator: SimulatorConfig
    network: NetworkConfig
    node: NodeConfig
    system: SystemConfig
    
    @model_validator(mode='after')
    def compute_round_length(self):
        if self.system.round_length is None:
            self.system.round_length = max(
                self.network.delay_bound,
                self.node.exec_bound
            ) + 10  # Adding a small buffer
            logger.info(f"Computed round_length: {self.system.round_length} ns")
        else:
            logger.info(f"Using provided round_length: {self.system.round_length} ns")
            
            theoretical_min = self.network.delay_bound + self.node.exec_bound
            if self.system.round_length < theoretical_min:
                raise ValueError(
                    f"Provided round_length {self.system.round_length} ns is less than "
                    f"theoretical minimum {theoretical_min} ns"
                )
        return self

def load_config(config_path: str = "./sim_conf.toml") -> Config:
    path = Path(config_path)
    if not path.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_path}")
    
    with path.open("rb") as f:
        config_data = tomllib.load(f)

    try:
        config = Config(**config_data)
        logger.info("Configuration loaded successfully")
        return config
    except ValidationError as e:
        logger.error(f"Configuration validation error: {e}")
        sys.exit(1)
        
if __name__ == "__main__":
    config = load_config()
    logger.info(f"Loaded configuration: {config.model_dump_json(indent=2)}")
    logger.info("Configuration loaded successfully.")
