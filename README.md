# PeerConnect

Peer-to-peer C++ library for network

- Network connection **over** firewall or NAT
- No ip address and port number required
- Support NAT traversal and [WebRTC](https://webrtc.org)


## Quick start
Peer A (listen)
```c++
PeerConnect pc("PEER_A");

pc.On("message", function_pc(string peer, PeerConnect::Buffer& data) {
  std::cout << "A message has been received." << std::endl;
});

pc.Open();
```

Peer B (connect)
```c++
PeerConnect pc("PEER_B");

pc.On("open", function_pc(string peer) {
  pc.>Connect("PEER_A");
});

pc.On("connect", function_pc(string peer) {
  pc.Send("PEER_A", "Hello");
});

pc.Open();
```

### Example

See the **examples** directory.

#### [p2p netcat](https://github.com/peersio/peerconnect/tree/master/examples/p2p_netcat): netcat on peer-to-peer way

```
Usage: pnc [-l] name

  Options:
    -l      Listen mode, for inbound connections

Example:
  > pnc -l random_id      : Listen randoom_id
  > pnc random_id         : Connect to random_id
```


## Build

The currently supported platforms are Windows, Mac OS X and Linux.

#### Before You Start

First, be sure to get WebRTC source and install prerequisite software.

1. In the [WebRTC development](https://webrtc.org/native-code/development/) instruction:
* Follow 'Before You Start' step
* Follow 'Getting the code' step
2. Currently PeerConnect supports branch-heads/54 so run:
```
$ git checkout -b peers54 refs/remotes/branch-heads/54
```
3. To update build toolchain and dependencies of WebRTC, run:
```
$ gclient sync
```

Note that you don't have to follow 'Generating Ninja project files' or 'Compiling' step. PeerConnect will do that internally.


#### Build PeerConnect

1. Generate the build files
```
$ cd /path/to/src
$ mkdir build
$ cd build
$ cmake .. -DWEBRTC_ROOT_DIR=/path/to/webrtc-checkout/src \
      -DDEPOT_TOOLS_DIR=/path/to/depot_tools
```
**NOTE** On windows, you can generate Visual Studio 2015 project.
```
$ cmake -G "Visual Studio 14 2015" .. \
      -DWEBRTC_ROOT_DIR=/path/to/webrtc-checkout/src \
      -DDEPOT_TOOLS_DIR=/path/to/depot_tools
```
2. Finally you can build generated makefile.
```
$ make
```


#### Prerequisit software

**Windows**
* You must have Visual Studio 2015 Update 2.
* You must have Windows7 x64 or later. x86 OSs are unsupported
* CMake 2.8 or later

**Mac OS X**
* Xcode5 or later
* CMake 3.2 or later

**Linux**
* We except you will have the most luck on Ubuntu 14.04
* CMake 2.8 or later



## API


