# Paxos Protocol Simulation in Data Center Networks using NS-3

## Project Description
This project implements a simulation of the Paxos consensus protocol in data center network environments using the NS-3 network simulator. It models the behavior of distributed systems achieving consensus under various network conditions.

## Key Features
- Complete implementation of Paxos roles: Proposers, Acceptors, and Learners
- Network-level simulation of message passing with realistic delays
- Configurable network topologies (fat-tree, leaf-spine, etc.)
- Multiple failure scenarios (node crashes, network partitions)
- Performance metrics collection (latency, throughput, message counts)

## Prerequisites
- NS-3 (version 3.45 recommended)
- CMake (version 3.12 or higher)
- C++17 compatible compiler (GCC 9+, Clang 10+)
- Python 3 (for analysis scripts)

## Installation
```bash
git clone [repository-url]
cd SyncDC
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Basic Usage
Run the simulation with default parameters:
```bash
./sync-paxos
```

Customize simulation parameters:
```bash
./sync-paxos --nodes=5 --topology=leaf-spine --failures=2 --duration=60
```

## Configuration
Modify `config/simulation_config.json` to:
- Adjust Paxos timeouts
- Change network characteristics
- Set logging levels
- Specify metrics collection

## Project Structure
```
SyncDC/
├── CMakeLists.txt          - Main build configuration
├── ns-3.45/                - NS-3 simulator
├── sync-paxos/             - Core Paxos implementation
│   ├── CMakeLists.txt      - Module build config
│   ├── include/            - Header files
│   └── src/               - Implementation files
├── config/                 - Simulation configurations
└── build/                  - Build directory
```

## Contributing
1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Open a pull request

## License
[MIT License] - See LICENSE file for details

## Contact
For questions or support: [your-email@example.com]