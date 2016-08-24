/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include "control.h"
#include "peer.h"

#include "webrtc/base/json.h"
#include "webrtc/base/signalthread.h"

#include "logging.h"

#ifdef WEBRTC_POSIX
#include "webrtc/base/messagehandler.h"
#include "webrtc/base/messagequeue.h"

namespace rtc {

  MessageHandler::~MessageHandler() {
    MessageQueueManager::Clear(this);
  }

} // namespace rtc
#endif // WEBRTC_POSIX

namespace pc {

Control::Control()
       : Control(nullptr){
}

Control::Control(std::shared_ptr<Signal> signal)
       : signal_(signal) {

  signal_->SignalOnCommandReceived_.connect(this, &Control::OnSignalCommandReceived);
  signal_->SignalOnClosed_.connect(this, &Control::OnSignalConnectionClosed);
  LOGP_F( INFO ) << "Done";
}

Control::~Control() {
  LOGP_F( INFO ) << "Starting";

  peers_.clear();
  DeleteControl();
  signal_->SignalOnCommandReceived_.disconnect(this);
  signal_->SignalOnClosed_.disconnect(this);

  LOGP_F( INFO ) << "Done";
}


//
// Initialization and release
//

bool Control::InitializeControl() {

  ASSERT(peer_connection_factory_.get() == NULL);

  webrtc::MediaConstraintsInterface* constraints = NULL;

  if (!CreatePeerFactory(constraints)) {
    LOGP_F(LERROR) << "CreatePeerFactory failed";
    DeleteControl();
    return false;
  }

  webrtc_thread_ = rtc::Thread::Current();
  ASSERT( webrtc_thread_ != nullptr);

  return true;
}

void Control::DeleteControl() {
  LOGP_F( INFO ) << "Starting";

  peer_connection_factory_ = NULL;
  fake_audio_capture_module_ = NULL;

  LOGP_F( INFO ) << "Done";
}


void Control::SignIn(const string& user_id, const string& user_password, const string& open_id) {
  // 1. Connect to signal server
  // 2. Send signin command to signal server
  // 3. Send createchannel command to signal server (channel name is id or alias)
  //    Other peers connect to this peer by channel name, that is id or alias
  // 4. Generate 'signedin' event to PeerConnect

  if (signal_.get() == NULL) {
    LOGP_F( LERROR ) << "SignIn failed, no signal server";
    return;
  }

  open_id_ = open_id;
  user_id_ = user_id;

  // Start by signing in
  signal_->SignIn(user_id, user_password);

  LOGP_F( INFO ) << "Done";
  return;
}

void Control::SignOut() {

  if (webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(0, ref_);
    webrtc_thread_->Post(this, MSG_SIGNOUT, data);
    return;
  }

  signal_->SignOut();
  Close();

  LOGP_F( INFO ) << "Done";
}

void Control::Connect(const string id) {
  // 1. Join channel on signal server
  // 2. Server(remote) peer createoffer
  // 3. Client(local) peer answeroffer
  // 4. Connect datachannel

  if (signal_.get() == NULL) {
    LOGP_F(LERROR) << "Join failed, no signal server";
    return;
  }

  LOGP_F( INFO ) << "Joining channel " << id;
  JoinChannel(id);
}

void Control::Close(const string id, bool force_queuing) {

  //
  // Called by 
  //  PeerConnect if user closes the connection.
  //  PeerControl if remote peer closes a ice connection or data channel
  //

  if (force_queuing || webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(id, ref_);
    webrtc_thread_->Post(this, MSG_CLOSE_PEER, data);
    return;
  }

  // 1. Leave channel on signal server
  // 2. Erase peer
  // 3. Close peer

  LeaveChannel(id);

  auto peer_found = peers_.find(id);
  if ( peer_found == peers_.end() ) {
    LOGP_F( WARNING ) << "peer not found, " << id;
    return;
  }

  Peer peer = peer_found->second;
  peers_.erase( peer_found );
  peer->Close();

  LOGP_F( INFO ) << "Done, id is " << id;
}

void Control::Close( const string id ) {
  Close( id, QUEUEING_ON );
}

void Control::Close() {
  std::vector<string> peer_ids;

  for (auto peer : peers_) {
    peer_ids.push_back(peer.second->remote_id());
  }

  LOGP_F(INFO) << "Close(): peer count is " << peer_ids.size();

  for (auto id : peer_ids) {
    LOGP_F( INFO ) << "Try to close peer having id " << id;
    Close(id, QUEUEING_ON);
  }
}

//
// Send data to peer
//

void Control::Send(const string to, const char* buffer, const size_t size) {

  typedef std::map<string, rtc::scoped_refptr<PeerControl>>::iterator it_type;

  it_type it = peers_.find(to);
  if (it == peers_.end()) return;

  it->second->Send(buffer, size);
  return;
}

bool Control::SyncSend(const string to, const char* buffer, const size_t size) {

  typedef std::map<string, rtc::scoped_refptr<PeerControl>>::iterator it_type;

  it_type it = peers_.find(to);
  if (it == peers_.end()) return false;

  return it->second->SyncSend(buffer, size);
}


//
// Send command to other peer by signal server
//

void Control::SendCommand(const string& id, const string& command, const Json::Value& data) {
  signal_->SendCommand(id, command, data);
}


void Control::OnConnected(const string id) {
  if ( observer_ == nullptr ) {
    LOGP_F( WARNING ) << "observer_ is null, id is " << id;
    return;
  }

  observer_->OnPeerConnected(id);
  LOGP_F( INFO ) << "Done, id is " << id;
}

//
// Ice connection state has been changed to close.
// It means that peer data channel had been closed already.
//
// Implements PeerObserver::OnDisconnected()
//

void Control::OnClosed(const string id, const bool force_queuing) {

  if (force_queuing || webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(id, ref_);

    // Call Control::OnPeerDisconnected()
    webrtc_thread_->Post(this, MSG_ON_PEER_CLOSED, data);
    LOGP_F( INFO ) << "Queued, id is " << id;
    return;
  }

 
  if ( observer_ == nullptr ) {
    LOGP_F( WARNING ) << "observer_ is null, id is " << id;
    return;
  }

  LOGP_F( INFO ) << "Calling OnPeerDisconnected, id is " << id;
  observer_->OnPeerDisconnected(id);

  if (peers_.size() == 0) {
    LOGP_F( INFO ) << "peers_ has been empty. id is " << id;
    OnSignedOut(open_id_);
  }

  LOGP_F( INFO ) << "Done, id is " << id;
}

void Control::OnClosed(const string id) {
  OnClosed( id, QUEUEING_ON );
}


//
// Signal receiving data
//

void Control::OnMessage(const string& id, const char* buffer, const size_t size) {
  if ( observer_ == nullptr ) {
    LOGP_F( WARNING ) << "observer_ is null, id is " << id;
    return;
  }
  observer_->OnPeerMessage(id, buffer, size);
}

void Control::OnWritable(const string& id) {
  if ( observer_ == nullptr ) {
    LOGP_F( WARNING ) << "observer_ is null, id is " << id;
    return;
  }
  observer_->OnPeerWritable(id);
}

void Control::OnError( const string id, const string& reason ) {
  if ( observer_ == nullptr ) {
    LOGP_F( WARNING ) << "observer_ is null, id is " << id;
    return;
  }
  observer_->OnError( id, reason );
}


void Control::RegisterObserver(ControlObserver* observer, std::shared_ptr<Control> ref) {
  ref_ = ref;
  observer_ = observer;

  LOGP_F( INFO ) << "Registered";
}

void Control::UnregisterObserver() {
  observer_ = nullptr;
  ref_.reset();

  LOGP_F( INFO ) << "Unregistered";
}

//
// Thread message queue
//

void Control::OnMessage(rtc::Message* msg) {
  ControlMessageData* param = nullptr;
    switch (msg->message_id) {
  case MSG_COMMAND_RECEIVED:
    param = static_cast<ControlMessageData*>(msg->pdata);
    OnCommandReceived(param->data_json_);
    break;
  case MSG_CLOSE_PEER:
    param = static_cast<ControlMessageData*>(msg->pdata);
    Close(param->data_string_, QUEUEING_OFF);
    break;
  case MSG_ON_PEER_CLOSED:
    param = static_cast<ControlMessageData*>(msg->pdata);
    OnClosed(param->data_string_, QUEUEING_OFF);
    break;
  case MSG_SIGNOUT:
    param = static_cast<ControlMessageData*>(msg->pdata);
    SignOut();
    break;
  case MSG_SIGNAL_SERVER_CLOSED:
    param = static_cast<ControlMessageData*>(msg->pdata);
    OnSignedOut(param->data_string_);
    break;
  default:
    break;
  }

  if (param != nullptr) delete param;
  return;
}

//
// Dispatch command from signal server
//

void Control::OnCommandReceived(const Json::Value& message) {

  Json::Value data;
  string command;
  string peer_id;

  if (!rtc::GetStringFromJsonObject(message, "command", &command) ||
      !rtc::GetValueFromJsonObject(message, "data", &data)) {

    LOGP_F(LERROR) << "Invalid message:" << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(message, "peer_id", &peer_id)) {
    peer_id.clear();
  }

  if (command == "signin") {
    OnSignedIn(data);
  }
  else if (command == "channelcreated") {
    OnChannelCreated(data);
  }
  else if (command == "channeljoined") {
    OnChannelJoined(data);
  }
  else if (command == "channelleaved") {
    OnChannelLeaved(data);
  }
  else if (command == "createoffer") {
    CreateOffer(data);
  }
  else if (command == "offersdp") {
    ReceiveOfferSdp(peer_id, data);
  }
  else if (command == "answersdp") {
    ReceiveAnswerSdp(peer_id, data);
  }
  else if (command == "ice_candidate") {
    AddIceCandidate(peer_id, data);
  }
}

void Control::OnSignalCommandReceived(const Json::Value& message) {
  ControlMessageData *data = new ControlMessageData(message, ref_);
  webrtc_thread_->Post(this, MSG_COMMAND_RECEIVED, data);
  LOGP_F( INFO ) << "Done";
}

void Control::OnSignalConnectionClosed(websocketpp::close::status::value code) {
  LOGP_F(INFO) << "Calling OnSignalConnectionClosed() with " << code;
  if (code == websocketpp::close::status::normal) {
    ControlMessageData *data = new ControlMessageData(open_id_, ref_);
    webrtc_thread_->Post(this, MSG_SIGNAL_SERVER_CLOSED, data);
  }
  LOGP_F( INFO ) << "Done";
}

void Control::OnSignedOut(const string& id) {
  LOGP_F( INFO ) << "Calling OnSignedOut() with " << id;

  if ( signal_ == nullptr || signal_->opened() ) {
    LOGP_F( WARNING ) << "signal_ is null or not opened";
    return;
  }

  if ( peers_.size() != 0 ) {
    LOGP_F( WARNING ) << "peers_ is empty";
    return;
  }

  if ( observer_ != nullptr ) {
    observer_->OnSignedOut( id );
  }

  LOGP_F( INFO ) << "Done";
}


//
// Commands to signal server
//

void Control::CreateChannel(const string name) {
  LOGP_F( INFO ) << "channel is " << name;

  Json::Value data;
  data["name"] = name;
  SendCommand(name, "createchannel", data);
}

void Control::JoinChannel(const string name) {
  LOGP_F( INFO ) << "channel is " << name;

  Json::Value data;
  data["name"] = name;
  SendCommand(name, "joinchannel", data);
}

void Control::LeaveChannel(const string name) {
  LOGP_F( INFO ) << "channel is " << name;

  Json::Value data;
  data["name"] = name;
  SendCommand(name, "leavechannel", data);
}


//
// Create peer creation factory
//

bool Control::CreatePeerFactory(
  const webrtc::MediaConstraintsInterface* constraints) {

  fake_audio_capture_module_ = FakeAudioCaptureModule::Create();
  if (fake_audio_capture_module_ == NULL) {
    LOGP_F( LERROR ) << "Failed to create FakeAudioCaptureModule";
    return false;
  }

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
    rtc::Thread::Current(), rtc::Thread::Current(),
    fake_audio_capture_module_, NULL, NULL);

  if (!peer_connection_factory_.get()) {
    LOGP_F( LERROR ) << "Failed to create CreatePeerConnectionFactory";
    return false;
  }

  LOGP_F( INFO ) << "Done";
  return true;
}


//
// Add ice candidate to local peer from remote peer
//

void Control::AddIceCandidate(const string& peer_id, const Json::Value& data) {

  string sdp_mid;
  int sdp_mline_index;
  string candidate;

  if ( !rtc::GetStringFromJsonObject( data, "sdp_mid", &sdp_mid ) ) {
    LOGP_F( LERROR ) << "sdp_mid not found, " << data.toStyledString();
    return;
  }

  if ( !rtc::GetIntFromJsonObject( data, "sdp_mline_index", &sdp_mline_index ) ) {
    LOGP_F( LERROR ) << "sdp_mline_index not found, " << data.toStyledString();
    return;
  }

  if ( !rtc::GetStringFromJsonObject( data, "candidate", &candidate ) ) {
    LOGP_F( LERROR ) << "candidate not found, " << data.toStyledString();
    return;
  }

  if ( peers_.find( peer_id ) == peers_.end() ) {
    LOGP_F( WARNING ) << "peer_id not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  peers_[peer_id]->AddIceCandidate(sdp_mid, sdp_mline_index, candidate);
  LOGP_F( INFO ) << "Done, peer_id is " << peer_id;
}



//
// 'signin' command
//

void Control::OnSignedIn(const Json::Value& data) {
  bool result;
  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOGP_F(WARNING) << "Unknown signin response";
    return;
  }

  if (!result) {
    LOGP_F(LERROR) << "Signin failed";
    return;
  }

  string session_id;
  if (!rtc::GetStringFromJsonObject(data, "session_id", &session_id)) {
    LOGP_F(LERROR) << "Signin failed - no session_id";
    return;
  }

  session_id_ = session_id;

  //
  // Create channel
  //

  CreateChannel(open_id_);
  LOGP_F( INFO ) << "Done";
}


void Control::OnChannelCreated(const Json::Value& data) {
  bool result;
  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOGP_F(WARNING) << "Unknown signin response";
    return;
  }

  string channel;
  if (!rtc::GetStringFromJsonObject(data, "name", &channel)) {
    LOGP_F(LERROR) << "Create channel failed - no channel name";
    return;
  }

  if (!result) {
    LOGP_F(LERROR) << "Create channel failed";
    string reason;
    if (!rtc::GetStringFromJsonObject(data, "reason", &reason)) {
      reason = "Unknown reason";
    }
    observer_->OnError(channel, reason);
    return;
  }

  observer_->OnSignedIn(channel);
  LOGP_F( INFO ) << "Done";
}

void Control::OnChannelJoined(const Json::Value& data) {
  bool result;

  LOGP_F(INFO) << "OnChannelJoined(" << data.toStyledString() << ")";

  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOGP_F(LERROR) << "Unknown channel join response";
    return;
  }

  string channel;
  if (!rtc::GetStringFromJsonObject(data, "name", &channel)) {
    LOGP_F(LERROR) << "Join channel failed - no channel name";
    return;
  }

  if (!result) {
    LOGP_F(LERROR) << "Join channel failed";
    string reason;
    if (!rtc::GetStringFromJsonObject(data, "reason", &reason)) {
      reason = "Unknown reason";
    }
    observer_->OnError(channel, reason);
    return;
  }

  LOGP_F( INFO ) << "Done";
}


//
// 'leave' command
//

void Control::OnChannelLeaved(const Json::Value& data) {
  // Nothing
}


//
// 'createoffer' command
//

void Control::CreateOffer(const Json::Value& data) {

  Json::Value peers;
  if (!rtc::GetValueFromJsonObject(data, "peers", &peers)) {
    LOGP_F(LERROR) << "createoffer failed - no peers value";
    return;
  }

  for (size_t i = 0; i < peers.size(); ++i) {
    string remote_id;
    if (!rtc::GetStringFromJsonArray(peers, i, &remote_id)) {
      LOGP_F(LERROR) << "Peer handshake failed - invalid peer id";
      return;
    }

    Peer peer = new rtc::RefCountedObject<PeerControl>(open_id_, remote_id, this, peer_connection_factory_);
    if ( !peer->Initialize() ) {
      LOGP_F( LERROR ) << "Peer initialization failed";
      OnClosed( remote_id );
      return;
    }

    peers_.insert(std::pair<string, Peer>(remote_id, peer));
    peer->CreateOffer(NULL);
  }

  LOGP_F( INFO ) << "Done";
}

//
// 'offersdp' command
//

void Control::ReceiveOfferSdp(const string& peer_id, const Json::Value& data) {
  string sdp;

  if ( !rtc::GetStringFromJsonObject( data, "sdp", &sdp ) ) {
    LOGP_F( LERROR ) << "sdp not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  Peer peer = new rtc::RefCountedObject<PeerControl>(open_id_, peer_id, this, peer_connection_factory_);
  if ( !peer->Initialize() ) {
    LOGP_F( LERROR ) << "Peer initialization failed";
    OnClosed( peer_id );
    return;
  }

  peers_.insert(std::pair<string, Peer>(peer_id, peer));
  peer->ReceiveOfferSdp(sdp);

  LOGP_F( INFO ) << "Done";
}


//
// 'answersdp' command
//

void Control::ReceiveAnswerSdp(const string& peer_id, const Json::Value& data) {
  string sdp;

  if ( !rtc::GetStringFromJsonObject( data, "sdp", &sdp ) ) {
    LOGP_F( LERROR ) << "sdp not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  auto peer = peers_.find(peer_id);
  if ( peer == peers_.end() ) {
    LOGP_F( LERROR ) << "peer_id not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  peer->second->ReceiveAnswerSdp(sdp);
  LOGP_F( INFO ) << "Done";
}

} // namespace pc
