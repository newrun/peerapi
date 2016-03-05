/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "config.h"
#include "throughnet.h"


Throughnet::Throughnet(std::string id, std::string setting) {

}


Throughnet::~Throughnet() {
  return;
}

bool Throughnet::Connect(std::string id, std::string setting) {

  return true;
}
 	
void Throughnet::Send(std::string id, std::string data) {
  
  return;
}


Throughnet& Throughnet::On(std::string name, void(*handler) (Throughnet* this_, Data& data)) {

  if (name == "connected") {
    events_["connected"] = handler;
  }
  else if (name == "disconnected") {
    events_["disconnected"] = handler;
  }
  else if (name == "data") {
    events_["data"] = handler;
  }
  else if (name == "error") {
    events_["error"] = handler;
  }
  else {
    // Do nothing
  }

  return *this;
}
