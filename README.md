# ThroughNet

Throughnet is a network library for peer to peer (serverless) network programming.

- Network connection by unique id or your email
- Support NAT traversal and WebRTC
- Forget ip address and port number

### How to use
Peer A (listen)
```
  Throughnet tn("your@email");

  tn.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    std::cout << "A message has been received." << std::endl;
  });
 ```
Peer B (connect)
```
Throughnet tn;

tn.On("ready", function_tn(Throughnet* tn, string id) {
  tn->Connect("your@email");
});

tn.On("connected", function_tn(Throughnet* tn, string id) {
  tn->Send("your@email", "Hello world");
});
```

You can make your own application with ThroughnNet
- Connect VR game players without server costs
- Connect IoT device and smart phone
- Connect people who want to share big file or data
- Connect every single device and network (home/office network, printer, pc, phone, ..)

### How it works


### How to build


### Support
