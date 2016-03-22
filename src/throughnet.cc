/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "config.h"
#include "throughnet.h"
#include "signal.h"
#include "dummysignal.h"

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

  control_ = new rtc::RefCountedObject<Control>(channel, signal_);

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Run failed, no control";
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

  if (!control_->InitializePeerConnection()) {
    LOG(LS_ERROR) << "Run failed, InitializePeerConnection failed";
    control_->DeletePeerConnection();
    return;
  }

  //
  // Connect to signal server
  //

  control_->SignIn();
  return;
}


bool Throughnet::Send(const char* message) {
  return Send(std::string(message));
}

bool Throughnet::Send(const std::string& message) {
  return control_->Send(message);
}

Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_sid, Data& data)) {

  if (msg_id == "connected") {
    events_[msg_id] = handler;
  }
  else if (msg_id == "disconnected") {
    events_[msg_id] = handler;
  }
  else if (msg_id == "signin") {
    events_[msg_id] = handler;
  }
  else if (msg_id == "signout") {
    events_[msg_id] = handler;
  }
  else if (msg_id == "error") {

  }

  return *this;
}

Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_sid, Buffer& data)) {

  if (msg_id.length() > 0) {
    data_handler_[msg_id] = handler;
  }

  return *this;
}

void Throughnet::OnConnected(std::string& peer_sid) {
  if (events_.find("connected") == events_.end()) return;

  Data data;
  events_["connected"](this, peer_sid, data);
}

void Throughnet::OnData(const std::string& channel_name, const char* buffer, const size_t size) {
  if (data_handler_.find(channel_name) == data_handler_.end()) return;

  Buffer buf(buffer, size);
  data_handler_[channel_name](this, "", buf);
}

