/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "config.h"
#include "throughnet.h"


Throughnet::Throughnet()
   : Throughnet("", nullptr){
}

Throughnet::Throughnet(const std::string setting)
   : Throughnet(setting, nullptr) {
}

Throughnet::Throughnet(const std::string setting, rtc::scoped_refptr<Signal> signal)
   : signal_(signal)
{
  if (signal == nullptr) signal_ = new rtc::RefCountedObject<tn::Signal>();
}

Throughnet::~Throughnet() {
}


void Throughnet::Connect(const std::string channel) {

  //
  // Initialize control
  //

  control_ = new rtc::RefCountedObject<Control>(channel, signal_);

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Failed to create class Control.";
    return;
  }

  //
  // connect sigslot
  //

  control_->SignalOnConnected_.connect(this, &Throughnet::OnConnected);
  control_->SignalOnData_.connect(this, &Throughnet::OnData);

  //
  // Initialize peer connection
  //

  if (!control_->InitializeControl()) {
    LOG(LS_ERROR) << "Failed to initialize Control.";
    return;
  }

  //
  // Connect to signal server
  //

  control_->SignIn();
  return;
}


//
// Send message to destination peer
// destination: channel name or peer session_id
//

bool Throughnet::Send(const std::string& destination, const char* message) {
  return Send(destination, std::string(message));
}

bool Throughnet::Send(const std::string& destination, const std::string& message) {
  if (control_->channel_name() == destination) {
    return control_->Send(message);
  }

  return false;
}

//
// Register Event handler
//

Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_sid, Data& data)) {

  if (msg_id == "connected") {
    event_handler_[msg_id] = handler;
  }
  else if (msg_id == "disconnected") {
    event_handler_[msg_id] = handler;
  }
  else if (msg_id == "signin") {
    event_handler_[msg_id] = handler;
  }
  else if (msg_id == "signout") {
    event_handler_[msg_id] = handler;
  }
  else if (msg_id == "error") {
    event_handler_[msg_id] = handler;
  }

  return *this;
}


//
// Register Date handler
//

Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_sid, Buffer& data)) {

  if (msg_id.length() > 0) {
    data_handler_[msg_id] = handler;
  }

  return *this;
}

//
// Signal event handler
//

void Throughnet::OnConnected(const std::string& channel, const std::string& peer_sid) {
  if (event_handler_.find("connected") == event_handler_.end()) return;

  Data data;
  data["channel"] = channel;
  event_handler_["connected"](this, peer_sid, data);
}

//
// Signal data handler
//

void Throughnet::OnData(const std::string& channel, const std::string& peer_id, const char* buffer, const size_t size) {
  if (data_handler_.find(channel) == data_handler_.end()) return;

  Buffer buf(buffer, size);
  data_handler_[channel](this, peer_id, buf);
}

