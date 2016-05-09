/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include <iostream>
#include <string>

#include "throughnet.h"

using namespace std;
void usage(const char* prg);


int main(int argc, char *argv[]) {

  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }

  string name = argv[1];

  Throughnet tn;

  tn.On("ready", function_tn(Throughnet* tn, string id) {
    tn->Connect(name);
  });

  tn.On("connected", function_tn(Throughnet* tn, string id) {
    std::cout << "Peer " << id << " has been connected." << std::endl;
    tn->Send(id, "Hello world");
  });

  tn.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    std::cout << "Message " << std::string(data.buf_, data.size_) << 
                 " has been received." << std::endl;
  });

  tn.GetReady();
  Throughnet::Run();

  return 0;
}

void usage(const char* prg) {
  std::cerr << std::endl;
  std::cerr << "Usage: " << prg << " id" << std::endl << std::endl;
  std::cerr << "Example: " << std::endl << std::endl;
  std::cerr << "   > " << prg << " echo@myserver.com" << std::endl;
}