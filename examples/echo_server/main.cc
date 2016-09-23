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
  string server = argv[1];

  PeerConnect pc(server);

  pc.On("connect", function_pc(string peer) {
    std::cout << "Peer " << peer << " has been connected." << std::endl;
  });

  pc.On("close", function_pc(string peer, CloseCode code, string desc) {
    std::cout << "Peer " << peer << " has been closed." << std::endl;
  });

  pc.On("message", function_pc(string peer, char* data, size_t size) {
    std::cout << "Message " << std::string(data, size) << 
                 " has been received." << std::endl;
    pc.Send(peer,data, size);
  });

  pc.Open();
  PeerConnect::Run();

  return 0;
}

void usage(const char* prg) {
  std::cerr << std::endl;
  std::cerr << "Usage: " << prg << " name" << std::endl << std::endl;
  std::cerr << "Example: " << std::endl << std::endl;
  std::cerr << "   > " << prg << " peername" << std::endl;
}