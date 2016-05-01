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

  Throughnet tn1;
  Throughnet tn2;
  
  tn2.On("connected", function_tn(Throughnet* tn, string peer_id) {
    std::cout << "Peer " << peer_id << " has been connected." << std::endl;
    tn->Emit("mychannel", "hello");
  });

  tn1.On("mychannel", function_tn(Throughnet* tn, string peer_id, Throughnet::Buffer& data) {
    std::cout << "Message " << std::string(data.buf_, data.size_) << " has been received." << std::endl;
  });
  
  tn1.Connect("mychannel");
  tn2.Connect("mychannel");
  
  Throughnet::Run();

  return 0;
}

