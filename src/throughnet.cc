/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "config.h"
#include "throughnet.h"
#include "signal.h"
#include "dummysignal.h"

Throughnet::Throughnet(const std::string channel)
   : Throughnet(channel, "", nullptr){
}

Throughnet::Throughnet(const std::string channel, const std::string setting)
   : Throughnet(channel, setting, nullptr) {
}

Throughnet::Throughnet(const std::string channel, const std::string setting, rtc::scoped_refptr<Signal> signal)
    : channel_(channel)
{
  rtc::scoped_refptr<Signal> localsignal = (signal != nullptr ? signal : new rtc::RefCountedObject<tn::Signal>());
  control_ = new rtc::RefCountedObject<Control>(channel, localsignal);

  control_->SignalOnConnected_.connect(this, &Throughnet::OnConnected);
  control_->SignalOnData_.connect(this, &Throughnet::OnData);
}

Throughnet::~Throughnet() {

}

void Throughnet::Start() {

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Run failed, no control";
    return;
  }

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

void Throughnet::OnData(const char* buffer, const size_t size) {
  if (data_handler_.find(channel_) == data_handler_.end()) return;

  Buffer buf(buffer, size);
  data_handler_[channel_](this, "", buf);
}

