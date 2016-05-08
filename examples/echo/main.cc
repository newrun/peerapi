/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include <iostream>
#include <string>

#include "throughnet.h"

using namespace std;


int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " yourname" << std::endl;
    return 1;
  }

  string name = argv[1];
  size_t count = 0;

  Throughnet tn;

  tn.On("connect", function_tn(Throughnet* tn, string peer_id) {
    tn->Send(peer_id, "Ping");
  });

  tn.OnData(name, function_tn(Throughnet* tn, string peer_id, Throughnet::Buffer& data) {
    std::string message(data.buf_, data.size_);
    std::cout << "Message " << message << " has been received." << std::endl;
    
    if (count == 0) {
      tn->Send(peer_id, "Pong");
      count++;
    }
  });

  tn.Connect(name);
  Throughnet::Run();

  return 0;
}
