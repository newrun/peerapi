# ThroughNet

A Throughnet is a P2P network socket library.

- Network connection by random id or email address
- No ip address and port number
- Support NAT traversal and WebRTC


### How to use
Peer A (listen)
```
Throughnet tn;
tn.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
  std::cout << "A message has been received." << std::endl;
});
tn.SignIn("PEER_A");
 ```

Peer B (connect)
```
Throughnet tn;
tn.On("signin", function_tn(Throughnet* tn, string id) {
  tn->Connect("PEER_A");
});
tn.On("connect", function_tn(Throughnet* tn, string id) {
  tn->Send("PEER_A", "Hello");
});
tn.SignIn("PEER_B");
```

### How it works

### How to build
