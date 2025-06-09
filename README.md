# Synchronous Data Center Network Simulator

This project is a simulator that can be used for synchronous data center network simulation.

## Project Structure

```
SyncDC/
├── CMakeLists.txt          - Main build configuration
├── ns-3.45/                - NS-3 simulator
├── sync-paxos/             - Core Paxos implementation
│   ├── CMakeLists.txt      - Module build config
│   ├── include/            - Header files
│   └── src/                - Implementation files
└── build/                  - Build directory
```

## Installation
```bash
git clone [repository-url]
cd SyncDC
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Contact
For questions or support: [mayuke803@gmail.com]
