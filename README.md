# mclear
Adaptive request routing based on load on a node in the cluster

The project consists of client and server. The client is meant to run on the nodes as daemon.
The clients will send the vital statistics about the load on the particular node to the server.
Based on this information, the server will then decide which node among the cluster has least load and accordingly route the new requests.

![Alt text](mclear.gif "schematic diagram")
