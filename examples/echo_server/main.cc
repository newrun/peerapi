/*
*  Copyright 2016 The PeerApi Project Authors. All rights reserved.
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

  Peer peer(server);

  peer.On("connect", function_peer(string peer_id) {
    std::cout << "Peer " << peer_id << " has been connected." << std::endl;
  });

  peer.On("close", function_peer(string peer_id, CloseCode code, string desc) {
    std::cout << "Peer " << peer_id << " has been closed." << std::endl;
  });

  peer.On("message", function_peer(string peer_id, char* data, size_t size) {
    std::cout << "Message " << std::string(data, size) << 
                 " has been received." << std::endl;
    peer.Send(peer_id,data, size);
  });

  peer.Open();
  Peer::Run();

  return 0;
}

void usage(const char* prg) {
  std::cerr << std::endl;
  std::cerr << "Usage: " << prg << " name" << std::endl << std::endl;
  std::cerr << "Example: " << std::endl << std::endl;
  std::cerr << "   > " << prg << " peername" << std::endl;
}