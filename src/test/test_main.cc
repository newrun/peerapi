/*
*  Copyright 2016 The PeerApi Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include <iostream>
#include <string>
#include <thread>
#include <cassert>

#include "peerapi.h"

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

  std::string server_id = Peer::CreateRandomUuid();
  std::string client_id = Peer::CreateRandomUuid();

  Peer peer1(server_id);
  Peer peer2(client_id);

  peer1.On("open", function_peer( string peer_id ) {
    assert(peer_id == server_id);
    std::cout << "peer1: open" << std::endl;
    peer2.Open();
  });

  peer1.On("connect", function_peer( string peer_id ) {
    assert(peer_id == client_id);
    std::cout << "peer1: peer2(" <<peer_id << ") connected" << std::endl;
  });

  peer1.On("close", function_peer( string peer_id, CloseCode code, string desc ) {
    assert(peer_id == client_id || peer_id == server_id);
    if ( peer_id == client_id ) {
      std::cout << "peer1: peer2 disconnected" << std::endl;
    }
    else if ( peer_id == server_id ) {
      std::cout << "peer1: close out" << std::endl;
      peer2.Close();
    }
  });

  peer1.On("message", function_peer( string peer_id, char* data, size_t size ) {
    assert(std::string(data, size) == "Ping");
    assert(peer_id == client_id);
    std::cout << "peer1: a message has been received" << std::endl;
    peer1.Send(client_id, "Pong");
  });


  peer2.On("open", function_peer( string peer_id ) {
    assert(peer_id == client_id);
    std::cout << "peer2: open" << std::endl;
    peer2.Connect(server_id);
  });

  peer2.On("connect", function_peer( string peer_id ) {
    assert(peer_id == server_id);
    std::cout << "peer2: peer1(" << peer_id << ") connected" << std::endl;
    peer2.Send(server_id, "Ping");
  });

  peer2.On("close", function_peer( string peer_id, CloseCode code, string desc ) {
    assert( peer_id == server_id || peer_id == client_id );
    if ( peer_id == server_id ) {
      std::cout << "peer2: peer1 disconnected" << std::endl;
      peer1.Close();
    }
    else if ( peer_id == client_id ) {
      std::cout << "peer2: close out" << std::endl;
      Peer::Stop();
    }
  });

  peer2.On("message", function_peer( string peer_id, char* data, size_t size ) {
    assert(std::string(data, size) == "Pong");
    assert(peer_id == server_id);
    std::cout << "peer2 has received message" << std::endl;
    peer2.Close(server_id);
  });

  peer1.Open();
  Peer::Run();
}


void test_writable() {

  std::string server_id = Peer::CreateRandomUuid();
  std::string client_id = Peer::CreateRandomUuid();

  Peer peer1(server_id);
  Peer peer2(client_id);

  peer1.On("open", function_peer( string peer_id ) {
    assert(peer_id == server_id);
    std::cout << "peer1: open" << std::endl;
    peer2.Open();
  });

  peer1.On("connect", function_peer( string peer_id ) {
    assert(peer_id == client_id);
    std::cout << "peer1: peer2(" << peer_id << ") connected" << std::endl;
  });

  peer1.On("close", function_peer( string peer_id, CloseCode code, string desc ) {
    assert( peer_id == client_id || peer_id == server_id );
    if ( peer_id == client_id ) {
      std::cout << "peer1: peer2 disconnected" << std::endl;
    }
    else if ( peer_id == server_id ) {
      std::cout << "peer1: close" << std::endl;
      peer2.Close();
    }
  });

  peer1.On("writable", function_peer( string peer_id ){
    assert(peer_id == server_id);
    std::cout << "peer1: writable" << std::endl;
  });

  peer1.On("message", function_peer( string peer_id, char* data, size_t size ) {
    assert(std::string(data, size) == "Ping");
    assert(peer_id == client_id);
    std::cout << "peer1: a message has been received" << std::endl;
    peer1.Send(client_id, "Pong");
  });


  peer2.On("open", function_peer( string peer_id ) {
    assert(peer_id == client_id);
    std::cout << "peer2: open" << std::endl;
    peer2.Connect(server_id);
  });

  peer2.On("connect", function_peer( string peer_id ) {
    assert( peer_id == server_id );
    std::cout << "peer2: peer1(" << peer_id << ") connected" << std::endl;
  });

  peer2.On("close", function_peer( string peer_id, CloseCode code, string desc ) {
    assert( peer_id == server_id || peer_id == client_id);
    if ( peer_id == server_id ) {
      std::cout << "peer2: peer1 disconnected" << std::endl;
      peer1.Close();
    }
    else if ( peer_id == client_id ) {
      std::cout << "peer2: close" << std::endl;
      Peer::Stop();
    }
  });

  peer2.On("writable", function_peer( string peer_id ){
    assert(peer_id == client_id);
    std::cout << "peer2: writable" << std::endl;
    peer2.Send(server_id, "Ping");
  });

  peer2.On("message", function_peer( string peer_id, char* data, size_t size ) {
    assert(std::string(data, size) == "Pong");
    assert(peer_id == server_id);
    std::cout << "peer2 has received message" << std::endl;
    peer2.Close(server_id);
  });

  peer1.Open();
  Peer::Run();

}


