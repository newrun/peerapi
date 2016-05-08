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

Control::Control(ControlObserver* observer, const std::string channel)
       : Control(observer, channel, nullptr){
}

Control::Control(ControlObserver* observer, const std::string channel, std::shared_ptr<Signal> signal)
       : observer_(observer), channel_name_(channel),
         signal_(signal) {

  signal_->SignalOnCommandReceived_.connect(this, &Control::OnSignalCommandReceived);
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
  signal_->SendCommand(peer_sid, command, data);
  return true;
}


//
// Signal connected
//

void Control::OnConnected(const std::string peer_id) {
  if (observer_ == nullptr) return;
  observer_->OnConnected(channel_name_, peer_id);
}

//
// Signal receiving data
//

void Control::OnData(const std::string& peer_id, const char* buffer, const size_t size) {
  if (observer_ == nullptr) return;
  observer_->OnData(channel_name_, peer_id, buffer, size);
}

//
// Thread message queue
//

void Control::OnMessage(rtc::Message* msg) {
  switch (msg->message_id) {
  case MSG_COMMAND_RECEIVED:
    ControlMessageData* param =
      static_cast<ControlMessageData*>(msg->pdata);
    OnCommandReceived(param->data_);
    delete param;
    break;
  }

  return;
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
// Dispatch command from signal server
//

void Control::OnCommandReceived(const Json::Value& message) {

  Json::Value data;
  std::string command;
  std::string peer_sid;

  if (!rtc::GetStringFromJsonObject(message, "command", &command) ||
      !rtc::GetValueFromJsonObject(message, "data", &data)) {

    LOG(LS_ERROR) << "Invalid message:" << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(message, "peer_sid", &peer_sid)) {
    peer_sid.clear();
  }

  if (command == "signin") {
    OnSignedIn(data);
  }
  else if (command == "join") {
    OnJoined(data);
  }
  else if (command == "createoffer") {
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

void Control::OnSignalCommandReceived(const Json::Value& message) {
  ControlMessageData *data = new ControlMessageData(message);
  signaling_thread_->Post(this, MSG_COMMAND_RECEIVED, data);
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
// 'signin' command
//

void Control::OnSignedIn(const Json::Value& data) {
  bool result;
  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOG(LS_WARNING) << "Unknown signin response";
    return;
  }

  if (!result) {
    LOG(LS_WARNING) << "Signin failed";
    return;
  }

  std::string session_id;
  if (!rtc::GetStringFromJsonObject(data, "session_id", &session_id)) {
    LOG(LS_WARNING) << "Signin failed - no session_id";
    return;
  }

  session_id_ = session_id;
  signal_->JoinChannel(channel_name_);
}


//
// 'join' command
//

void Control::OnJoined(const Json::Value& data) {
}


//
// 'createoffer' command
//

void Control::CreateOffer(const Json::Value& data) {

  Json::Value peers;
  if (!rtc::GetValueFromJsonObject(data, "peers", &peers)) {
    LOG(LS_WARNING) << "createoffer failed - no peers value";
    return;
  }

  for (size_t i = 0; i < peers.size(); ++i) {
    std::string remote_sid;
    if (!rtc::GetStringFromJsonArray(peers, i, &remote_sid)) {
      LOG(LS_WARNING) << "Peer handshake failed - invalid peer sid";
      return;
    }

    Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, remote_sid, true, this, peer_connection_factory_);
    peers_.insert(std::pair<std::string, Peer>(remote_sid, peer));

    peer->CreateOffer(NULL);
  }
}

//
// 'offersdp' command
//

void Control::ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data) {
  std::string sdp;

  if (!rtc::GetStringFromJsonObject(data, "sdp", &sdp)) return;

  Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, peer_sid, false, this, peer_connection_factory_);
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
