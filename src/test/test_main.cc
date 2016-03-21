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
#include "dummysignal.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/api/datachannelinterface.h"
#include "webrtc/api/test/mockpeerconnectionobservers.h"

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

    /**
    webrtc::DataChannelInit init;
    rtc::scoped_refptr<webrtc::DataChannelInterface> caller_dc(
      caller_->CreateDataChannel("data", init));
    rtc::scoped_refptr<webrtc::DataChannelInterface> callee_dc(
      callee_->CreateDataChannel("data", init));
*/
    Negotiate();
    WaitForConnection();

    caller_->TestWaitForChannelOpen(10000);
    callee_->TestWaitForChannelOpen(10000);

    std::string msg = "test_meesage_from_caller";
    caller_->Send(msg);
    callee_->TestWaitForMessage(msg, 10000);
    callee_->Send(msg);
    caller_->TestWaitForMessage(msg, 10000);

    caller_->TestWaitForClose(10000);
    callee_->TestWaitForClose(10000);

    caller_ = NULL;
    callee_ = NULL;

    rtc::ThreadManager::Instance()->CurrentThread()->Run();


    /*
    WaitForDataChannelsToOpen(caller_dc, callee_signaled_data_channels_, 0);
    WaitForDataChannelsToOpen(callee_dc, caller_signaled_data_channels_, 0);
    
    TestDataChannelSendAndReceive(caller_dc, callee_signaled_data_channels_[0]);
    TestDataChannelSendAndReceive(callee_dc, caller_signaled_data_channels_[0]);

    CloseDataChannels(caller_dc, callee_signaled_data_channels_, 0);
    CloseDataChannels(callee_dc, caller_signaled_data_channels_, 0);
    */
  }

  void CreatePcs() {
    CreatePcs(NULL);
  }

  void CreatePcs(const webrtc::MediaConstraintsInterface* pc_constraints) {
    caller_->InitializePeerConnection();
    callee_->InitializePeerConnection();
//    caller_->CreatePc(pc_constraints);
//    callee_->CreatePc(pc_constraints);
    Control::Connect(caller_.get(), callee_.get());

//    caller_->SignalOnDataChannel.connect(
//        this, &ControlTest::OnCallerAddedDataChanel);
//    callee_->SignalOnDataChannel.connect(
//        this, &ControlTest::OnCalleeAddedDataChannel);
  }

  void Negotiate() {
    caller_->CreateOffer(NULL);
  }

  void WaitForConnection() {
    caller_->TestWaitForConnection(10000);
    callee_->TestWaitForConnection(10000);
  }

//  void OnCallerAddedDataChanel(webrtc::DataChannelInterface* dc) {
//    caller_signaled_data_channels_.push_back(dc);
//  }

//  void OnCalleeAddedDataChannel(webrtc::DataChannelInterface* dc) {
//    callee_signaled_data_channels_.push_back(dc);
//  }

  void WaitForDataChannelsToOpen(webrtc::DataChannelInterface* local_dc,
                                 const DataChannelList& remote_dc_list,
                                 size_t remote_dc_index) {
    WAIT_(webrtc::DataChannelInterface::kOpen == local_dc->state(), 10000);

    WAIT_(remote_dc_list.size() > remote_dc_index, 10000);
    WAIT_(webrtc::DataChannelInterface::kOpen == remote_dc_list[remote_dc_index]->state(),
          10000);
  }

  // Tests that |dc1| and |dc2| can send to and receive from each other.
  void TestDataChannelSendAndReceive(
        webrtc::DataChannelInterface* dc1, webrtc::DataChannelInterface* dc2) {
    rtc::scoped_ptr<webrtc::MockDataChannelObserver> dc1_observer(
      new webrtc::MockDataChannelObserver(dc1));

    rtc::scoped_ptr<webrtc::MockDataChannelObserver> dc2_observer(
      new webrtc::MockDataChannelObserver(dc2));

    static const std::string kDummyData = "abcdefg";
    webrtc::DataBuffer buffer(kDummyData);
    
    dc1->Send(buffer);
    WAIT_(kDummyData == dc2_observer->last_message(), 10000);

    std::string test1 = dc2_observer->last_message();

    dc2->Send(buffer);
    WAIT_(kDummyData == dc1_observer->last_message(), 10000);

    std::string test2 = dc1_observer->last_message();
  }

  void CloseDataChannels(webrtc::DataChannelInterface* local_dc,
                         const DataChannelList& remote_dc_list,
                         size_t remote_dc_index) {
    local_dc->Close();
    WAIT_(webrtc::DataChannelInterface::kClosed == local_dc->state(), 10000);
    WAIT_(webrtc::DataChannelInterface::kClosed == remote_dc_list[remote_dc_index]->state(), 10000);
  }


private:
  rtc::scoped_refptr<Control> caller_;
  rtc::scoped_refptr<Control> callee_;
  DataChannelList caller_signaled_data_channels_;
  DataChannelList callee_signaled_data_channels_;
};

} // namespace tn


int main(int argc, char *argv[]) {

  rtc::scoped_refptr<DummySignal> signal1(new rtc::RefCountedObject<DummySignal>());
  rtc::scoped_refptr<DummySignal> signal2(new rtc::RefCountedObject<DummySignal>());

  Throughnet tn1("mychannel", "", signal1);
  Throughnet tn2("mychannel", "", signal2);

  tn2.On("connected", function_tn(string peer_sid, Throughnet::Data& data) {
    std::cout << "Peer " << peer_sid << " has been connected." << std::endl;
    this_->Send("hello");
  });

  tn1.On("mychannel", function_tn(string peer_sid, Throughnet::Buffer& data) {
    std::cout << "Message " << std::string(data.buf_, data.size_) << " has been received." << std::endl;
  });

  tn1.Start();
  tn2.Start();

  rtc::ThreadManager::Instance()->CurrentThread()->Run();


//  ControlTest test;
//  test.Start();

  return 0;
}