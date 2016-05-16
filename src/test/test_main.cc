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

  std::string server_name = Throughnet::CreateRandomUuid();
  std::string client_name = Throughnet::CreateRandomUuid();

  Throughnet tn1(server_name);
  Throughnet tn2(client_name);

  tn1.On("ready", function_tn(Throughnet* tn, string id) {
    std::cout << "tn1: ready" << std::endl;
    tn2.GetReady();
  });

  tn1.On("connected", function_tn(Throughnet* tn, string id) {
    assert(id == client_name);
    std::cout << "tn1: tn2 connected" << std::endl;
  });

  tn1.On("disconnected", function_tn(Throughnet* tn, string id) {
    assert(id == client_name);
    std::cout << "tn1: tn2 disconnected" << std::endl;
  });
  
  tn1.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Ping");
    assert(id == client_name);
    std::cout << "tn1: a message has been received" << std::endl;
    tn->Send(client_name, "Pong");
  });


  tn2.On("ready", function_tn(Throughnet* tn, string id) {
    std::cout << "tn2: ready" << std::endl;
    tn->Connect(server_name);
  });

  tn2.On("connected", function_tn(Throughnet* tn, string id) {
    assert(id == server_name);
    std::cout << "tn2: tn1 connected" << std::endl;
    tn->Send(server_name, "Ping");
  });

  tn2.On("disconnected", function_tn(Throughnet* tn, string id) {
    assert(id == server_name);
    std::cout << "tn2: tn1 disconnected" << std::endl;
    Throughnet::Stop();
  });

  tn2.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Pong");
    assert(id == server_name);
    std::cout << "tn2 has received message" << std::endl;
    tn->Disconnect(server_name);
  });

  tn1.GetReady();  
  Throughnet::Run();

  std::cout << "Exit test" << std::endl;
  return 0;
}

