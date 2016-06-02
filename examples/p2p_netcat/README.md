# P2P NetCat

Peer to peer netcat connection

- Netcat with p2p connection
- Connection through different internal network
- No port forwarding required

### Usage
```
Usage: p2pnc [-l] name

  Options:
    -l      Listen mode, for inbound connections

Example:
  > p2pnc -l random_id      : Listen randoom_id
  > p2pnc random_id         : Connect to random_id
```

### Example

#### 1. Anonymous chat

Peer A (listen)
```
> p2pnc -l peer_a
Listening peer_a
Connected
Hello~
```
Peer B (connect)
```
> p2pnc peer_a
Connecting to peer_a
Connected
Hello~
```

#### 2. Send and receive file

Quickly send and receive file between your devices without cloud server.

Peer A (receive)
```
> p2pnc -l peer_a > dst_file.txt
```
Peer B (send)
```
> p2pnc peer_a < src_file.txt
```
