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

  PeerConnect pc1(server);
  PeerConnect pc2(client);

  pc1.On("open", function_pc( string peer ) {
    assert(peer == server);
    std::cout << "pc1: open" << std::endl;
    pc2.Open();
  });

  pc1.On("connect", function_pc( string peer ) {
    assert(peer == client);
    std::cout << "pc1: pc2(" << peer << ") connected" << std::endl;
  });

  pc1.On("close", function_pc( string peer, CloseCode code, string desc ) {
    assert(peer == client || peer == server);
    if ( peer == client ) {
      std::cout << "pc1: pc2 disconnected" << std::endl;
    }
    else if ( peer == server ) {
      std::cout << "pc1: close out" << std::endl;
      pc2.Close();
    }
  });

  pc1.On("message", function_pc( string peer, char* data, size_t size ) {
    assert(std::string(data, size) == "Ping");
    assert(peer == client);
    std::cout << "pc1: a message has been received" << std::endl;
    pc1.Send(client, "Pong");
  });


  pc2.On("open", function_pc( string peer ) {
    assert(peer == client);
    std::cout << "pc2: open" << std::endl;
    pc2.Connect(server);
  });

  pc2.On("connect", function_pc( string peer ) {
    assert(peer == server);
    std::cout << "pc2: pc1(" << peer << ") connected" << std::endl;
    pc2.Send(server, "Ping");
  });

  pc2.On("close", function_pc( string peer, CloseCode code, string desc ) {
    assert( peer == server || peer == client );
    if ( peer == server ) {
      std::cout << "pc2: pc1 disconnected" << std::endl;
      pc1.Close();
    }
    else if ( peer == client ) {
      std::cout << "pc2: close out" << std::endl;
      PeerConnect::Stop();
    }
  });

  pc2.On("message", function_pc( string peer, char* data, size_t size ) {
    assert(std::string(data, size) == "Pong");
    assert(peer == server);
    std::cout << "pc2 has received message" << std::endl;
    pc2.Close(server);
  });

  pc1.Open();
  PeerConnect::Run();
}


void test_writable() {

  std::string server = PeerConnect::CreateRandomUuid();
  std::string client = PeerConnect::CreateRandomUuid();

  PeerConnect pc1(server);
  PeerConnect pc2(client);

  pc1.On("open", function_pc( string peer ) {
    assert(peer == server);
    std::cout << "pc1: open" << std::endl;
    pc2.Open();
  });

  pc1.On("connect", function_pc( string peer ) {
    assert(peer == client);
    std::cout << "pc1: pc2(" << peer << ") connected" << std::endl;
  });

  pc1.On("close", function_pc( string peer, CloseCode code, string desc ) {
    assert( peer == client || peer == server );
    if ( peer == client ) {
      std::cout << "pc1: pc2 disconnected" << std::endl;
    }
    else if ( peer == server ) {
      std::cout << "pc1: close" << std::endl;
      pc2.Close();
    }
  });

  pc1.On("writable", function_pc( string peer ){
    assert(peer == server);
    std::cout << "pc1: writable" << std::endl;
  });

  pc1.On("message", function_pc( string peer, char* data, size_t size ) {
    assert(std::string(data, size) == "Ping");
    assert(peer == client);
    std::cout << "pc1: a message has been received" << std::endl;
    pc1.Send(client, "Pong");
  });


  pc2.On("open", function_pc( string peer ) {
    assert(peer == client);
    std::cout << "pc2: open" << std::endl;
    pc2.Connect(server);
  });

  pc2.On("connect", function_pc( string peer ) {
    assert( peer == server );
    std::cout << "pc2: pc1(" << peer << ") connected" << std::endl;
  });

  pc2.On("close", function_pc( string peer, CloseCode code, string desc ) {
    assert( peer == server || peer == client);
    if ( peer == server ) {
      std::cout << "pc2: pc1 disconnected" << std::endl;
      pc1.Close();
    }
    else if ( peer == client ) {
      std::cout << "pc2: close" << std::endl;
      PeerConnect::Stop();
    }
  });

  pc2.On("writable", function_pc( string peer ){
    assert(peer == client);
    std::cout << "pc2: writable" << std::endl;
    pc2.Send(server, "Ping");
  });

  pc2.On("message", function_pc( string peer, char* data, size_t size ) {
    assert(std::string(data, size) == "Pong");
    assert(peer == server);
    std::cout << "pc2 has received message" << std::endl;
    pc2.Close(server);
  });

  pc1.Open();
  PeerConnect::Run();

}


