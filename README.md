# PeerConnect

A PeerConnect is a peer-to-peer library for network.

- Network connection by random id
- No ip address and port number
- Support NAT traversal and WebRTC


### How to use
Peer A (listen)
```
PeerConnect pc1("PEER_A");
pc1.On("message", function_pc(string peer, PeerConnect::Buffer& data) {
  std::cout << "A message has been received." << std::endl;
});
pc1.Open();
 ```

Peer B (connect)
```
PeerConnect pc2("PEER_B");
pc2.On("open", function_pc(string peer) {
  pc2.>Connect("PEER_A");
});
pc2.On("connect", function_pc(string peer) {
  pc2.Send("PEER_A", "Hello");
});
pc2.Open();
```

### How it works

### How to build
