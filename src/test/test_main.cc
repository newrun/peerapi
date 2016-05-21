/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include <iostream>
#include <string>
#include <thread>
#include <cassert>

#include "throughnet.h"

using namespace std;

int main(int argc, char *argv[]) {

  std::string server = Throughnet::CreateRandomUuid();
  std::string client = Throughnet::CreateRandomUuid();

  Throughnet tn1;
  Throughnet tn2;

  tn1.On("signedin", function_tn(Throughnet* tn, string id) {
    std::cout << "tn1: signedin" << std::endl;
    tn2.SignIn(client);
  });

  tn1.On("connected", function_tn(Throughnet* tn, string id) {
    assert(id == client);
    std::cout << "tn1: tn2 connected" << std::endl;
  });

  tn1.On("disconnected", function_tn(Throughnet* tn, string id) {
    assert(id == client);
    std::cout << "tn1: tn2 disconnected" << std::endl;
  });
  
  tn1.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Ping");
    assert(id == client);
    std::cout << "tn1: a message has been received" << std::endl;
    tn->Disconnect(client);
//    tn->Send(client, "Pong");
  });


  tn2.On("signedin", function_tn(Throughnet* tn, string id) {
    std::cout << "tn2: signedin" << std::endl;
    tn->Connect(server);
  });

  tn2.On("connected", function_tn(Throughnet* tn, string id) {
    assert(id == server);
    std::cout << "tn2: tn1 connected" << std::endl;
    tn->Send(server, "Ping");
  });

  tn2.On("disconnected", function_tn(Throughnet* tn, string id) {
    assert(id == server);
    std::cout << "tn2: tn1 disconnected" << std::endl;
    Throughnet::Stop();
  });

  tn2.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Pong");
    assert(id == server);
    std::cout << "tn2 has received message" << std::endl;
    tn->Disconnect(server);
  });

  tn1.SignIn(server);
  Throughnet::Run();

  std::cout << "Exit test" << std::endl;
  return 0;
}

