The objective of this project is to model, simulate, and analyze the Go-Back-N protocol. Go-Back-N protocol is based on sliding window flow control method.

At the start of the simulation, the TIC(source) should send a control message to TOC(destination) querying its window size, which is configured in the TOC at the time of initialization via the omnetpp.ini file. The TOC then replies to the query from TIC indicating the window size.

The query_reply message from TOC (indicating the Window size) should be used as a trigger by TIC to start sending the frames/packets. The TOC should then send a Receive Ready (i.e., RR message) or Receive Not Ready (RNR) message in case it receives or not receives frames transmitted from TIC.

The simulation model should be able to correctly demonstrate the operation of the Go Back N flow control mechanism in case of
  1. No packet loss or errors.
  2. In case of packet loss or errors.

Reference for the project:
--------------------------
1. Simulation manual of omnet++.
2. Tictoc tutorial provided on omnet++ official website.
