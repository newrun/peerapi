/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include <iostream>
#include <string>
#include <thread>
#include <cassert>

#include "peerconnect.h"

using namespace std;

void test_normal();
void test_writable();


int main(int argc, char *argv[]) {
  std::cout << "Start test" << std::endl;

//  test_normal();
  test_writable();

  std::cout << "Exit test" << std::endl;
  return 0;
}

void test_normal() {

  std::string server = PeerConnect::CreateRandomUuid();
  std::string client = PeerConnect::CreateRandomUuid();

  PeerConnect pc1;
  PeerConnect pc2;

  pc1.On("signin", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc1: signedin" << std::endl;
    pc2.SignIn(client);
  });

  pc1.On("connect", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc1: pc2(" << id << ") connected" << std::endl;
  });

  pc1.On("disconnect", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc1: pc2 disconnected" << std::endl;
  });

  pc1.On("signout", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc1: signed out" << std::endl;
    pc2.SignOut();
  });

  pc1.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Ping");
    assert(id == client);
    std::cout << "pc1: a message has been received" << std::endl;
    pc->Send(client, "Pong");
  });


  pc2.On("signin", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc2: signedin" << std::endl;
    pc->Connect(server);
  });

  pc2.On("connect", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc2: pc1(" << id << ") connected" << std::endl;
    pc->Send(server, "Ping");
  });

  pc2.On("disconnect", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc2: pc1 disconnected" << std::endl;
    pc1.SignOut();
  });

  pc2.On("signout", function_pc(PeerConnect* pc, string id){
    assert(id == client);
    std::cout << "pc2: signed out" << std::endl;
    PeerConnect::Stop();
  });

  pc2.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Pong");
    assert(id == server);
    std::cout << "pc2 has received message" << std::endl;
    pc->Disconnect(server);
  });

  pc1.SignIn(server);
  PeerConnect::Run();
}


void test_writable() {

  std::string server = PeerConnect::CreateRandomUuid();
  std::string client = PeerConnect::CreateRandomUuid();

  PeerConnect pc1;
  PeerConnect pc2;

  pc1.On("signin", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc1: signedin" << std::endl;
    pc2.SignIn(client);
  });

  pc1.On("connect", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc1: pc2(" << id << ") connected" << std::endl;
  });

  pc1.On("disconnect", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc1: pc2 disconnected" << std::endl;
  });

  pc1.On("writable", function_pc(PeerConnect* pc, string id){
    assert(id == server);
    std::cout << "pc1: writable" << std::endl;
  });

  pc1.On("signout", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc1: signed out" << std::endl;
    pc2.SignOut();
  });

  pc1.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Ping");
    assert(id == client);
    std::cout << "pc1: a message has been received" << std::endl;
    pc->Send(client, "Pong");
  });


  pc2.On("signin", function_pc(PeerConnect* pc, string id) {
    assert(id == client);
    std::cout << "pc2: signedin" << std::endl;
    pc->Connect(server);
  });

  pc2.On("connect", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc2: pc1(" << id << ") connected" << std::endl;
  });

  pc2.On("disconnect", function_pc(PeerConnect* pc, string id) {
    assert(id == server);
    std::cout << "pc2: pc1 disconnected" << std::endl;
    pc1.SignOut();
  });

  pc2.On("writable", function_pc(PeerConnect* pc, string id){
    assert(id == client);
    std::cout << "pc2: writable" << std::endl;
    pc->Send(server, "Ping");
  });

  pc2.On("signout", function_pc(PeerConnect* pc, string id){
    assert(id == client);
    std::cout << "pc2: signed out" << std::endl;
    PeerConnect::Stop();
  });

  pc2.OnMessage(function_pc(PeerConnect* pc, string id, PeerConnect::Buffer& data) {
    assert(std::string(data.buf_, data.size_) == "Pong");
    assert(id == server);
    std::cout << "pc2 has received message" << std::endl;
    pc->Disconnect(server);
  });

  pc1.SignIn(server);
  PeerConnect::Run();

}