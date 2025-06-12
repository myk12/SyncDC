# Synchrnous Paxos

Paxos is a commonly used consensus algorithm in distributed systems. The asynchronous nature of the network makes it difficult to implement a reliable while efficient Paxos implementation. Many works have shown that synchronous Paxos is more efficient than asynchronous Paxos.

This project is a synchronous Paxos implementation that based on a synchornous data center network.

## Network Topology

```mermaid
C4Container
    title Synchronous Paxos System

    Person(client, "Client", "Initiates requests.")

    System_Boundary(datacenter, "Synchronous Data Center Network") {
        Container(paxos_comm_hub, "Synchrnous Fabric", "Network Backbone", "Facilitates synchronized Paxos messages among servers.")
        Container(server1, "Server 1", "Paxos Node", "Participates")
        Container(server2, "Server 2", "Paxos Node", "Participates")
        Container(server3, "Server 3", "Paxos Node", "Participates")
        Container(server4, "Server 4", "Paxos Node", "Participates")
        Container(server5, "Server 5", "Paxos Node", "Participates")
    }

    Rel(client, paxos_comm_hub, "Request (to all servers)", "Synchronous RPC/Message")

    Rel(paxos_comm_hub, server1, "Paxos Communication", "Sync Messages")
    Rel(paxos_comm_hub, server2, "Paxos Communication", "Sync Messages")
    Rel(paxos_comm_hub, server3, "Paxos Communication", "Sync Messages")
    Rel(paxos_comm_hub, server4, "Paxos Communication", "Sync Messages")
    Rel(paxos_comm_hub, server5, "Paxos Communication", "Sync Messages")
```
