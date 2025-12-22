# Synchronous Replication State Machine Simulation

This directory contains a Python-based simulation of a synchronous replication state machine using the Paxos consensus algorithm. The simulation models a network of nodes that communicate with each other to achieve consensus on a series of values, simulating the behavior of a distributed system.

## Synchronous Protocol

The simulation implements a synchronous protocol where messages are guaranteed to be delivered within a known maximum delay. This is crucial for ensuring that all nodes can make progress in the consensus process without indefinite waiting.

```mermaid
sequenceDiagram
    autonumber
    participant N1 as Node 1
    participant N2 as Node 2
    participant N3 as Node 3

    Note over N1,N3: == Round 1: Proposal Exchange ==

    par Broadcast Proposals
        N1->>N2: Propose(Val_1)
        N1->>N3: Propose(Val_1)
    and
        N2->>N1: Propose(Val_2)
        N2->>N3: Propose(Val_2)
    and
        N3->>N1: Propose(Val_3)
        N3->>N2: Propose(Val_3)
    end

    Note over N1,N3: ⚙️ Local Action: Collect & Sort by Priority (e.g., Node ID)<br/>Result: Sequence [Val_1, Val_2, Val_3]

    Note over N1,N3: == Round 2: Sequence Verification ==

    par Broadcast Log Sequences
        N1->>N2: Send_Seq([V1, V2, V3])
        N1->>N3: Send_Seq([V1, V2, V3])
    and
        N2->>N1: Send_Seq([V1, V2, V3])
        N2->>N3: Send_Seq([V1, V2, V3])
    and
        N3->>N1: Send_Seq([V1, V2, V3])
        N3->>N2: Send_Seq([V1, V2, V3])
    end

    Note over N1,N3: ⚙️ Local Action: Compare Received Sequences

    rect rgb(200, 255, 200)
        Note over N1,N3: ✅ Commit Phase
        N1->>N1: Commit Logs if Sequences Match
        N2->>N2: Commit Logs if Sequences Match
        N3->>N3: Commit Logs if Sequences Match
    end
```


## Components

- `main.py`: The main entry point for the simulation. It sets up the simulator, network, and nodes, and runs the simulation.
- `sim_core/`: Contains the core simulation framework, including the `Simulator` and `PaxosNode` classes.
- `SyncNetwork`: A custom network class that simulates synchronous message delivery with a specified maximum delay.
- `sim_conf.toml`: Configuration file for the simulation, specifying parameters such as the number of nodes, round length, and random seed.

## Running the Simulation
To run the simulation, execute the `main.py` script. Ensure that you have Python installed along with any required dependencies.

```bash
python main.py
```
The simulation will initialize the specified number of Paxos nodes, set up the synchronous network, and run for a defined number of rounds. At the end of the simulation, each node will save its state and log the results.
