/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include <iostream>
#include <string>

#include "peerconnect.h"

using namespace std;
void usage(const char* prg);


int main(int argc, char *argv[]) {

  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }

  string name = argv[1];

  PeerConnect pc;

  pc.On("signin", function_pc(PeerConnect* pc, string id) {
    pc->Connect(name);
  });

  pc.On("connect", function_pc(PeerConnect* pc, string id) {
    pc->Send(id, "Hello world");
    std::cout << "Sent 'Hello world' message to " << id << "." << std::endl;
  });

  pc.On("disconnect", function_pc(PeerConnect* pc, string id) {
    std::cout << "Peer " << id << " has been disconnected" << std::endl;
    PeerConnect::Stop();
  });

  pc.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
    std::cout << "Message '" << std::string(data.buf_, data.size_) << 
                 "' has been received." << std::endl;
  });

  pc.SignIn();
  PeerConnect::Run();

  return 0;
}

void usage(const char* prg) {
  std::cerr << std::endl;
  std::cerr << "Usage: " << prg << " name" << std::endl << std::endl;
  std::cerr << "Example: " << std::endl << std::endl;
  std::cerr << "   > " << prg << " myrandom" << std::endl;
}