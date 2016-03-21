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

Throughnet::Throughnet(const std::string channel, const std::string setting, rtc::scoped_refptr<Signal> signal) {
  rtc::scoped_refptr<Signal> localsignal = (signal != nullptr ? signal : new rtc::RefCountedObject<tn::Signal>());
  control_ = new rtc::RefCountedObject<Control>(channel, localsignal);
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
