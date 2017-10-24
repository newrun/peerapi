/*
*  Copyright 2016 The PeerApi Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include "control.h"
#include "peer.h"

#include "webrtc/base/location.h"
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

namespace peerapi {

Control::Control()
       : Control(nullptr){
}

Control::Control(std::shared_ptr<Signal> signal)
       : signal_(signal) {

  signal_->SignalOnCommandReceived_.connect(this, &Control::OnSignalCommandReceived);
  signal_->SignalOnClosed_.connect(this, &Control::OnSignalConnectionClosed);
  LOG_F( INFO ) << "Done";
}

Control::~Control() {
  LOG_F( INFO ) << "Starting";

  peers_.clear();
  DeleteControl();
  signal_->SignalOnCommandReceived_.disconnect(this);
  signal_->SignalOnClosed_.disconnect(this);

  LOG_F( INFO ) << "Done";
}


//
// Initialization and release
//

bool Control::InitializeControl() {

  RTC_DCHECK(peer_connection_factory_.get() == NULL);

  webrtc::MediaConstraintsInterface* constraints = NULL;

  if ( !CreatePeerFactory(constraints) ) {
    LOG_F(LERROR) << "CreatePeerFactory failed";
    DeleteControl();
    return false;
  }

  webrtc_thread_ = rtc::Thread::Current();
  RTC_DCHECK( webrtc_thread_ != nullptr );

  return true;
}

void Control::DeleteControl() {
  LOG_F( INFO ) << "Starting";

  peer_connection_factory_ = NULL;
  fake_audio_capture_module_ = NULL;

  LOG_F( INFO ) << "Done";
}


void Control::Open(const string& user_id, const string& user_password, const string& peer_id) {

  // 1. Connect to signal server
  // 2. Send open command to signal server
  // 3. Send createchannel command to signal server.
  //    A peer name is user-supplied string to listen or random string.
  //    Other peers connect to this peer by peer name.
  // 4. Generate 'open' event to Peer

  if (signal_.get() == NULL) {
    LOG_F( LERROR ) << "Open failed, no signal server";
    return;
  }

  peer_name_ = peer_id;
  user_id_ = user_id;

  // Connect to signal server
  signal_->Open(user_id, user_password);

  LOG_F( INFO ) << "Done";
  return;
}

void Control::Connect(const string peer_id) {

  // 1. Join channel on signal server
  // 2. Server(remote) peer createoffer
  // 3. Client(local) peer answeroffer
  // 4. Connect datachannel

  if (signal_.get() == NULL) {
    LOG_F(LERROR) << "Join failed, no signal server";
    return;
  }

  LOG_F( INFO ) << "Joining channel " << peer_id;
  JoinChannel(peer_id);
}

void Control::Close(const CloseCode code, bool force_queuing) {

  LOG_F( INFO ) << "Call";

  //
  // Verify current thread
  //

  if (force_queuing || webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(code, ref_);
    webrtc_thread_->Post(RTC_FROM_HERE, this, MSG_CLOSE, data);
    LOG_F( INFO ) << "Queued";
    return;
  }

  //
  // Close peers
  //

  std::vector<string> peer_ids;

  for (auto peer : peers_) {
    peer_ids.push_back(peer.second->remote_id());
  }

  LOG_F(INFO) << "Close(): peer count is " << peer_ids.size();

  for (auto id : peer_ids) {
    LOG_F( INFO ) << "Try to close peer having id " << id;
    ClosePeer(id, code);
  }

  //
  // Close signal server
  //

  if ( peer_ ) {
    peer_->OnClose( peer_name_ ,code );
  }

  LOG_F( INFO ) << "Done";
}

void Control::ClosePeer( const string peer_id, const CloseCode code, bool force_queuing ) {

  //
  // Called by 
  //  Peer if user closes the connection.
  //  PeerControl if remote peer closes a ice connection or data channel
  //

  if (force_queuing || webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(peer_id, ref_);
    data->data_int32_ = code;
    webrtc_thread_->Post(RTC_FROM_HERE, this, MSG_CLOSE_PEER, data);
    return;
  }

  // 1. Erase peer
  // 2. Close peer

  auto peer_found = peers_.find(peer_id);
  if ( peer_found == peers_.end() ) {
    LOG_F( WARNING ) << "peer not found, " << peer_id;
    return;
  }

  Peer item = peer_found->second;
  peers_.erase( peer_found );
  item->Close(code);

  // 3. Leave channel on signal server
  LeaveChannel(peer_id);

  LOG_F( INFO ) << "Done, peer is " << peer_id;
}

//
// Send data to peer
//

void Control::Send(const string to, const char* data, const size_t size) {

  typedef std::map<string, rtc::scoped_refptr<PeerControl>>::iterator it_type;

  it_type it = peers_.find(to);
  if (it == peers_.end()) return;

  it->second->Send(data, size);
  return;
}

bool Control::SyncSend(const string to, const char* data, const size_t size) {

  typedef std::map<string, rtc::scoped_refptr<PeerControl>>::iterator it_type;

  it_type it = peers_.find(to);
  if (it == peers_.end()) return false;

  return it->second->SyncSend(data, size);
}


//
// Send command to other peer by signal server
//

void Control::SendCommand(const string& peer_id, const string& command, const Json::Value& data) {
  signal_->SendCommand(peer_id, command, data);
}


void Control::OnPeerConnect(const string peer_id) {
  if ( peer_ == nullptr ) {
    LOG_F( WARNING ) << "peer_ is null, peer is " << peer_id;
    return;
  }

  peer_->OnConnect(peer_id);
  LOG_F( INFO ) << "Done, peer is " << peer_id;
}

void Control::OnPeerClose(const string peer_id, CloseCode code) {

  if (webrtc_thread_ != rtc::Thread::Current()) {
    ControlMessageData *data = new ControlMessageData(peer_id, ref_);

    // Call Control::OnPeerDisconnected()
    webrtc_thread_->Post(RTC_FROM_HERE, this, MSG_ON_PEER_CLOSE, data);
    LOG_F( INFO ) << "Queued, peer is " << peer_id;
    return;
  }
 
  LOG_F( INFO ) << "Enter, peer is " << peer_id;

  if ( peer_ == nullptr ) {
    LOG_F( WARNING ) << "peer_ is null, peer is " << peer_id;
    return;
  }

  peer_->OnClose( peer_id, code );

  LOG_F( INFO ) << "Done, peer is " << peer_id;
}

//
// Signal receiving data
//

void Control::OnPeerMessage(const string& peer_id, const char* data, const size_t size) {
  if ( peer_ == nullptr ) {
    LOG_F( WARNING ) << "peer_ is null, peer is " << peer_id;
    return;
  }
  peer_->OnMessage(peer_id, data, size);
}

void Control::OnPeerWritable(const string& peer_id) {
  if ( peer_ == nullptr ) {
    LOG_F( WARNING ) << "peer_ is null, peer is " << peer_id;
    return;
  }
  peer_->OnWritable(peer_id);
}

void Control::RegisterObserver(ControlObserver* observer, std::shared_ptr<Control> ref) {
  ref_ = ref;
  peer_ = observer;

  LOG_F( INFO ) << "Registered";
}

void Control::UnregisterObserver() {
  peer_ = nullptr;
  ref_.reset();

  LOG_F( INFO ) << "Unregistered";
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
  case MSG_CLOSE:
    param = static_cast<ControlMessageData*>(msg->pdata);
    Close((CloseCode)param->data_int32_);
    break;
  case MSG_CLOSE_PEER:
    param = static_cast<ControlMessageData*>(msg->pdata);
    ClosePeer(param->data_string_, (CloseCode) param->data_int32_);
    break;
  case MSG_ON_PEER_CLOSE:
    param = static_cast<ControlMessageData*>(msg->pdata);
    OnPeerClose(param->data_string_, (CloseCode) param->data_int32_);
    break;
  case MSG_ON_SIGLAL_CONNECTION_CLOSE:
    param = static_cast<ControlMessageData*>(msg->pdata);
    Close((CloseCode)param->data_int32_);
    break;
  default:
    LOG_F( WARNING ) << "Unknown message";
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

    LOG_F(LERROR) << "Invalid message:" << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(message, "peer_id", &peer_id)) {
    peer_id.clear();
  }

  if (command == "open") {
    OnOpen(data);
  }
  else if (command == "channelcreate") {
    OnChannelCreate(data);
  }
  else if (command == "channeljoin") {
    OnChannelJoin(data);
  }
  else if (command == "channelleave") {
    OnChannelLeave(data);
  }
  else if (command == "peerclosed") {
    OnRemotePeerClose(peer_id, data);
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
  webrtc_thread_->Post(RTC_FROM_HERE, this, MSG_COMMAND_RECEIVED, data);
  LOG_F( INFO ) << "Done";
}

void Control::OnSignalConnectionClosed(websocketpp::close::status::value code) {
  LOG_F(INFO) << "Enter, code is " << code;
  if (code != websocketpp::close::status::normal) {
    ControlMessageData *data = new ControlMessageData(CLOSE_SIGNAL_ERROR, ref_);
    webrtc_thread_->Post(RTC_FROM_HERE, this, MSG_ON_SIGLAL_CONNECTION_CLOSE, data);
  }
  LOG_F( INFO ) << "Done";
}

//
// Commands to signal server
//

void Control::CreateChannel(const string name) {
  LOG_F( INFO ) << "channel is " << name;

  Json::Value data;
  data["name"] = name;
  SendCommand(name, "createchannel", data);
}

void Control::JoinChannel(const string name) {
  LOG_F( INFO ) << "channel is " << name;

  Json::Value data;
  data["name"] = name;
  SendCommand(name, "joinchannel", data);
}

void Control::LeaveChannel(const string name) {
  LOG_F( INFO ) << "channel is " << name;

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
    LOG_F( LERROR ) << "Failed to create FakeAudioCaptureModule";
    return false;
  }

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
    rtc::Thread::Current(), rtc::Thread::Current(),
    fake_audio_capture_module_, NULL, NULL);

  if (!peer_connection_factory_.get()) {
    LOG_F( LERROR ) << "Failed to create CreatePeerConnectionFactory";
    return false;
  }

  LOG_F( INFO ) << "Done";
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
    LOG_F( LERROR ) << "sdp_mid not found, " << data.toStyledString();
    return;
  }

  if ( !rtc::GetIntFromJsonObject( data, "sdp_mline_index", &sdp_mline_index ) ) {
    LOG_F( LERROR ) << "sdp_mline_index not found, " << data.toStyledString();
    return;
  }

  if ( !rtc::GetStringFromJsonObject( data, "candidate", &candidate ) ) {
    LOG_F( LERROR ) << "candidate not found, " << data.toStyledString();
    return;
  }

  if ( peers_.find( peer_id ) == peers_.end() ) {
    LOG_F( WARNING ) << "peer_id not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  peers_[peer_id]->AddIceCandidate(sdp_mid, sdp_mline_index, candidate);
  LOG_F( INFO ) << "Done, peer_id is " << peer_id;
}



//
// 'open' command
//

void Control::OnOpen(const Json::Value& data) {
  bool result;
  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOG_F(WARNING) << "Unknown open response";
    return;
  }

  if (!result) {
    LOG_F(LERROR) << "Open failed";
    return;
  }

  string session_id;
  if (!rtc::GetStringFromJsonObject(data, "session_id", &session_id)) {
    LOG_F(LERROR) << "Open failed - no session_id";
    return;
  }

  session_id_ = session_id;

  //
  // Create channel
  //

  CreateChannel(peer_name_);
  LOG_F( INFO ) << "Done";
}


void Control::OnChannelCreate(const Json::Value& data) {
  bool result;
  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    LOG_F(WARNING) << "Unknown open response";
    peer_->OnClose(peer_name_, CLOSE_SIGNAL_ERROR);
    return;
  }

  string peer_id;
  if (!rtc::GetStringFromJsonObject(data, "name", &peer_id)) {
    peer_->OnClose(peer_name_, CLOSE_SIGNAL_ERROR);
    LOG_F(LERROR) << "Create channel failed - no channel name";
    return;
  }

  if (!result) {
    LOG_F(LERROR) << "Create channel failed";
    string desc;
    if (!rtc::GetStringFromJsonObject(data, "desc", &desc)) {
      desc = "Unknown reason";
    }

    peer_->OnClose(peer_id, CLOSE_SIGNAL_ERROR, desc);
    return;
  }

  peer_->OnOpen(peer_id);
  LOG_F( INFO ) << "Done";
}

void Control::OnChannelJoin(const Json::Value& data) {
  bool result;

  LOG_F(INFO) << "OnChannelJoined(" << data.toStyledString() << ")";

  if (!rtc::GetBoolFromJsonObject(data, "result", &result)) {
    peer_->OnClose( "", CLOSE_SIGNAL_ERROR );
    LOG_F(LERROR) << "Unknown channel join response";
    return;
  }

  string peer_id;
  if (!rtc::GetStringFromJsonObject(data, "name", &peer_id)) {
    peer_->OnClose( "", CLOSE_SIGNAL_ERROR );
    LOG_F(LERROR) << "Join channel failed - no channel name";
    return;
  }

  if (!result) {
    LOG_F(LERROR) << "Join channel failed";
    string desc;
    if (!rtc::GetStringFromJsonObject(data, "desc", &desc)) {
      desc = "Unknown reason";
    }

    peer_->OnClose( peer_id, CLOSE_SIGNAL_ERROR, desc );
    return;
  }

  LOG_F( INFO ) << "Done";
}


//
// 'leave' command
//

void Control::OnChannelLeave(const Json::Value& data) {
  // Do nothing
}


void Control::OnRemotePeerClose(const string& peer_id, const Json::Value& data) {
  ClosePeer( peer_id, CLOSE_NORMAL );
}

//
// 'createoffer' command
//

void Control::CreateOffer(const Json::Value& data) {

  Json::Value peers;
  if (!rtc::GetValueFromJsonObject(data, "peers", &peers)) {
    LOG_F(LERROR) << "createoffer failed - no peers value";
    return;
  }

  for (size_t i = 0; i < peers.size(); ++i) {
    string remote_id;
    if (!rtc::GetStringFromJsonArray(peers, i, &remote_id)) {
      LOG_F(LERROR) << "Peer handshake failed - invalid peer id";
      return;
    }

    Peer peer = new rtc::RefCountedObject<PeerControl>(peer_name_, remote_id, this, peer_connection_factory_);
    if ( !peer->Initialize() ) {
      LOG_F( LERROR ) << "Peer initialization failed";
      OnPeerClose( remote_id, CLOSE_ABNORMAL );
      return;
    }

    peers_.insert(std::pair<string, Peer>(remote_id, peer));
    peer->CreateOffer(NULL);
  }

  LOG_F( INFO ) << "Done";
}

//
// 'offersdp' command
//

void Control::ReceiveOfferSdp(const string& peer_id, const Json::Value& data) {
  string sdp;

  if ( !rtc::GetStringFromJsonObject( data, "sdp", &sdp ) ) {
    LOG_F( LERROR ) << "sdp not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  Peer peer = new rtc::RefCountedObject<PeerControl>(peer_name_, peer_id, this, peer_connection_factory_);
  if ( !peer->Initialize() ) {
    LOG_F( LERROR ) << "Peer initialization failed";
    OnPeerClose( peer_id, CLOSE_ABNORMAL );
    return;
  }

  peers_.insert(std::pair<string, Peer>(peer_id, peer));
  peer->ReceiveOfferSdp(sdp);

  LOG_F( INFO ) << "Done";
}


//
// 'answersdp' command
//

void Control::ReceiveAnswerSdp(const string& peer_id, const Json::Value& data) {
  string sdp;

  if ( !rtc::GetStringFromJsonObject( data, "sdp", &sdp ) ) {
    LOG_F( LERROR ) << "sdp not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  auto peer = peers_.find(peer_id);
  if ( peer == peers_.end() ) {
    LOG_F( LERROR ) << "peer_id not found, peer_id is " << peer_id << " and " <<
                        "data is " << data.toStyledString();
    return;
  }

  peer->second->ReceiveAnswerSdp(sdp);
  LOG_F( INFO ) << "Done";
}

} // namespace peerapi
