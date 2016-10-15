# PeerConnect

A peer-to-peer C++ library for network.

- Network connection **over** firewall or NAT
- No ip address and port number required
- Support NAT traversal and [WebRTC](https://webrtc.org)


# Quick start
Peer A (listen)
```c++
PeerConnect pc("PEER_A");

pc.On("message", function_pc(string peer, char* data, size_t size) {
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

# API

See the [API.md](API.md) document.

# Example

See the **examples** directory.

* [p2p netcat](https://github.com/peersio/peerconnect/tree/master/examples/p2p_netcat): netcat on peer-to-peer way


# Build

The currently supported `build` platforms are Windows, Mac OS X and Linux.
* Visual Studio 2015 Update 2 or later on Windows. (Windows 7 x64 or later)
* Linux 64bit - Most luck on Ubuntu 14.04
* Xcode 5 or later on OS X - Latest Xcode is recommended
* CMake 2.8 or later (On OS X, CMake 3.2 or later)


## Before You Start

First, be sure to install prerequisite software and get the WebRTC source.

In the [WebRTC development](https://webrtc.org/native-code/development/) instruction

* Follow 'Before You Start' step
* Follow 'Getting the code' step

Currently PeerConnect supports WebRTC branch-heads/55 so run

```
$ git checkout -b peers55 refs/remotes/branch-heads/55
```

To update build toolchain and dependencies of WebRTC, run:
```
$ gclient sync
```

Note that you don't have to follow 'Generating Ninja project files' or 'Compiling' step. PeerConnect will do that internally.


## Build PeerConnect

Generate the build files
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
Finally you can build generated makefile.
```
$ make
```