/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "throughnet.h"
#include "control.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/api/datachannelinterface.h"

using namespace std;
using namespace tn;

namespace tn {

class ControlTest
    : public sigslot::has_slots<> {

public:
  typedef std::vector<rtc::scoped_refptr<webrtc::DataChannelInterface> >
      DataChannelList;

  ControlTest()
    : caller_(new rtc::RefCountedObject<Control>(
      "caller")),
    callee_(new rtc::RefCountedObject<Control>(
      "callee")) {
  }


  void Start() {

    CreatePcs();

    webrtc::DataChannelInit init;
    rtc::scoped_refptr<webrtc::DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("data", init));
    rtc::scoped_refptr<webrtc::DataChannelInterface> callee_dc(
      callee_->CreateDataChannel("data", init));

    Negotiate();
    WaitForConnection();
  }

  void CreatePcs() {
    CreatePcs(NULL);
  }

  void CreatePcs(const webrtc::MediaConstraintsInterface* pc_constraints) {
    caller_->CreatePc(pc_constraints);
    callee_->CreatePc(pc_constraints);
    Control::Connect(caller_.get(), callee_.get());

    caller_->SignalOnDataChannel.connect(
        this, &ControlTest::OnCallerAddedDataChanel);
    callee_->SignalOnDataChannel.connect(
        this, &ControlTest::OnCalleeAddedDataChannel);
  }

  void Negotiate() {
    caller_->CreateOffer(NULL);
  }

  void WaitForConnection() {
    caller_->WaitForConnection();
    callee_->WaitForConnection();
  }

  void OnCallerAddedDataChanel(webrtc::DataChannelInterface* dc) {
    caller_signaled_data_channels_.push_back(dc);
  }

  void OnCalleeAddedDataChannel(webrtc::DataChannelInterface* dc) {
    callee_signaled_data_channels_.push_back(dc);
  }

private:
  rtc::scoped_refptr<Control> caller_;
  rtc::scoped_refptr<Control> callee_;
  DataChannelList caller_signaled_data_channels_;
  DataChannelList callee_signaled_data_channels_;
};

} // namespace tn

int main(int argc, char *argv[]) {
  
  ControlTest test;
  test.Start();

  return 0;
}