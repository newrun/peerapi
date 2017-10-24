# Netcat over p2p network
This is PeerApi library example.

# Usage
```
Usage: pnc [-l] name

  Options:
    -l      Listen mode, for inbound connections

Example:
  > pnc -l random_id      : Listen randoom_id
  > pnc random_id         : Connect to random_id
```

# Example

### Anonymous chat

Peer A (listen)
```
$ p2pnc -l peer_a
Listening peer_a
Connected
Hello~
```
Peer B (connect)
```
$ p2pnc peer_a
Connecting to peer_a
Connected
Hello~
```

### Send and receive file

Quickly send and receive file between your devices directly.

Peer A (listen and receive)
```
$ pnc -l peer_a > peer.txt
```
Peer B (connect and send)
```
$ pnc peer_a < peer.txt
```
