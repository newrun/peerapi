/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "config.h"

#include <iostream>

#include "throughnet.h"
#include "control.h"
#include "dummysignal.h"
#include "webrtc/base/scoped_ref_ptr.h"

using namespace std;


int main(int argc, char *argv[]) {

//  rtc::scoped_refptr<tn::DummySignal> signal1(new rtc::RefCountedObject<tn::DummySignal>());
//  rtc::scoped_refptr<tn::DummySignal> signal2(new rtc::RefCountedObject<tn::DummySignal>());

//  Throughnet tn1("", signal1);
//  Throughnet tn2("", signal2);

  Throughnet tn1("{\"url\":\"https://127.0.0.1:5000\"}");
  Throughnet tn2("{\"url\":\"https://127.0.0.1:5000\"}");

  tn2.On("connected", function_tn(string peer_sid, Throughnet::Data& data) {
    std::cout << "Peer " << peer_sid << " has been connected." << std::endl;
    this_->Emit("mychannel", "hello");
  });

  tn1.On("mychannel", function_tn(string peer_sid, Throughnet::Buffer& data) {
    std::cout << "Message " << std::string(data.buf_, data.size_) << " has been received." << std::endl;
  });

  tn1.Connect("mychannel");
  tn2.Connect("mychannel");

  rtc::ThreadManager::Instance()->CurrentThread()->Run();

  return 0;
}

