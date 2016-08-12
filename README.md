# PeerConnect

A PeerConnect is a peer to peer socket library.

- Network connection by random id or email address
- No ip address and port number
- Support NAT traversal and WebRTC


### How to use
Peer A (listen)
```
PeerConnect pc;
pc.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
  std::cout << "A message has been received." << std::endl;
});
pc.SignIn("PEER_A");
 ```

Peer B (connect)
```
PeerConnect pc;
pc.On("signin", function_pc(PeerConnect* pc, string id) {
  pc->Connect("PEER_A");
});
pc.On("connect", function_pc(PeerConnect* pc, string id) {
  pc->Send("PEER_A", "Hello");
});
pc.SignIn("PEER_B");
```

### How it works

### How to build
