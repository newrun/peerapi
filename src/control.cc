/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "control.h"
#include "peer.h"

#include "webrtc/base/json.h"
#include "webrtc/base/signalthread.h"



namespace tn {

Control::Control(const std::string channel)
       : Control(channel, nullptr){
}

Control::Control(const std::string channel, rtc::scoped_refptr<Signal> signal)
       : channel_name_(channel),
         signal_(signal) {

  signal_->SignalOnSignedIn_.connect(this, &Control::OnSignedIn);
  signal_->SignalOnCommandReceived_.connect(this, &Control::OnCommandReceived);
}

Control::~Control() {
  peers_.clear();
}


//
// Initialization and release
//

bool Control::InitializeControl() {

  ASSERT(peer_connection_factory_.get() == NULL);

  webrtc::MediaConstraintsInterface* constraints = NULL;

  if (!CreatePeerFactory(constraints)) {
    LOG(LS_ERROR) << "CreatePeerFactory failed";
    DeleteControl();
    return false;
  }

  signaling_thread_ = rtc::Thread::Current();

  return true;
}

void Control::DeleteControl() {
  peer_connection_factory_ = NULL;
  fake_audio_capture_module_ = NULL;
}



//
// Send data to peer or emit data to channel
//

bool Control::Send(const char* buffer, const size_t size, const std::string *peer_id) {
  bool ret = true;

  typedef std::map<std::string, rtc::scoped_refptr<PeerControl>>::iterator it_type;
  for (it_type iterator = peers_.begin(); iterator != peers_.end(); iterator++) {
    if (peer_id == nullptr || iterator->second->remote_session_id() == *peer_id) {
      if (!iterator->second->Send(buffer, size)) {
        ret = false;
      }
    }
  }

  return ret;
}


//
// Send command to other peer by signal server
//

bool Control::SendCommand(const std::string& command, const Json::Value& data, const std::string& peer_sid) {
  Json::Value jmessage;

  jmessage["command"] = command;
  jmessage["data"] = data;

  if (!peer_sid.empty()) {
    jmessage["peer_sid"] = peer_sid;
  }

  return false;
//  return signal_->SendCommand(jmessage);
}


//
// Signal connected
//

void Control::OnConnected(const std::string peer_id) {
  SignalOnConnected_(channel_name_, peer_id);
}

//
// Signal receiving data
//

void Control::OnData(const std::string& peer_id, const char* buffer, const size_t size) {
  SignalOnData_(channel_name_, peer_id, buffer, size);
}

//
// Thread message queue
//

void Control::OnMessage(rtc::Message* msg) {
  LOG(LS_INFO) << "OnMessage";
}

//
// Signin to signal server
//

void Control::SignIn() {
  if (signal_.get() == NULL) {
    LOG(LS_ERROR) << "SignIn failed, no signal server";
    return;
  }

  signal_->SignIn();
  return;
}

//
// Connect to channel when signed in
//

void Control::OnSignedIn(const std::string& sid) {
  session_id_ = sid;
  
  signal_->JoinChannel(channel_name_);
}


//
// Dispatch command from signal server
//

void Control::OnCommandReceived(const std::string& message) {

  Json::Reader reader;
  Json::Value jmessage;
  Json::Value data;
  std::string command;
  std::string peer_sid;

  if (!reader.parse(message, jmessage)) {
    LOG(WARNING) << "Received unknown message: " << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(jmessage, "command", &command) ||
      !rtc::GetValueFromJsonObject(jmessage, "data", &data)) {

    LOG(LS_ERROR) << "Invalid message:" << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(jmessage, "peer_sid", &peer_sid)) {
    peer_sid.clear();
  }

  if (command == "createoffer") {
    CreateOffer(data);
  }
  else if (command == "offersdp") {
    ReceiveOfferSdp(peer_sid, data);
  }
  else if (command == "answersdp") {
    ReceiveAnswerSdp(peer_sid, data);
  }
  else if (command == "ice_candidate") {
    AddIceCandidate(peer_sid, data);
  }
}

//
// Create peer creation factory
//

bool Control::CreatePeerFactory(
  const webrtc::MediaConstraintsInterface* constraints) {

  fake_audio_capture_module_ = FakeAudioCaptureModule::Create();
  if (fake_audio_capture_module_ == NULL) {
    return false;
  }

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
    rtc::Thread::Current(), rtc::Thread::Current(),
    fake_audio_capture_module_, NULL, NULL);

  if (!peer_connection_factory_.get()) {
    return false;
  }

  return true;
}


//
// Add ice candidate to local peer from remote peer
//

void Control::AddIceCandidate(const std::string& peer_sid, const Json::Value& data) {

  std::string sdp_mid;
  int sdp_mline_index;
  std::string candidate;

  if (!rtc::GetStringFromJsonObject(data, "sdp_mid", &sdp_mid)) return;
  if (!rtc::GetIntFromJsonObject(data, "sdp_mline_index", &sdp_mline_index)) return;
  if (!rtc::GetStringFromJsonObject(data, "candidate", &candidate)) return;

  if (peers_.find(peer_sid) == peers_.end()) return;
  peers_[peer_sid]->AddIceCandidate(sdp_mid, sdp_mline_index, candidate);
}


//
// 'createoffer' command
//

void Control::CreateOffer(const Json::Value& data) {

  std::string peer_sid;
  if (!rtc::GetStringFromJsonObject(data, "peer_sid", &peer_sid)) {
    return;
  }

  Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, peer_sid, this, peer_connection_factory_);
  peers_.insert(std::pair<std::string, Peer>(peer_sid, peer));

  peer->CreateOffer(NULL);
}

//
// 'offersdp' command
//

void Control::ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data) {
  std::string sdp;

  if (!rtc::GetStringFromJsonObject(data, "sdp", &sdp)) return;

  Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, peer_sid, this, peer_connection_factory_);
  peers_.insert(std::pair<std::string, Peer>(peer_sid, peer));

  peer->ReceiveOfferSdp(sdp);
}


//
// 'answersdp' command
//

void Control::ReceiveAnswerSdp(const std::string& peer_sid, const Json::Value& data) {
  std::string sdp;

  if (!rtc::GetStringFromJsonObject(data, "sdp", &sdp)) return;
  if (peers_.find(peer_sid) == peers_.end()) return;

  peers_[peer_sid]->ReceiveAnswerSdp(sdp);
}


} // namespace tn
