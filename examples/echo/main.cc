/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include <iostream>
#include <string>

#include "throughnet.h"

using namespace std;

enum { max_length = 1024 };

static size_t count = 0;

int main(int argc, char *argv[]) {
  Throughnet tn;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " channelname" << std::endl;
    return 1;
  }

  string channel_name = argv[1];

  tn.On("connected", function_tn(Throughnet* tn, string peer_id) {
    tn->Emit("testchannel", "Ping");
  });

  tn.On(channel_name, function_tn(Throughnet* tn, string peer_id, Throughnet::Buffer& data) {
    std::string message(data.buf_, data.size_);
    std::cout << "Message " << message << " has been received." << std::endl;
    
    if (::count == 0) {
      tn->Emit("testchannel", "Pong");
      ::count++;
    }
  });

  tn.Connect(channel_name);

  Throughnet::Run();
  return 0;
}
