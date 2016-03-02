/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "config.h"
#include "throughnet.h"

Throughnet::Throughnet(void(*handler) (Throughnet* this_, std::string, tn::EventData&)) {
  handler_ = handler;
  return;
}

Throughnet::~Throughnet() {
  return;
}

void Throughnet::Connect(std::string id, std::string setting) {
  return;
}
 	
void Throughnet::Send(std::string id, std::string data) {
  
  return;
}

