/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#include <string>
#include <locale>
 
#include "peerconnect.h"
#include "control.h"
#include "logging.h"

PeerConnect::PeerConnect()
   : PeerConnect(""){
}

PeerConnect::PeerConnect(const std::string setting) {
  // Log level
#if DEBUG || _DEBUG
  rtc::LogMessage::LogToDebug(rtc::LS_NONE);
  pc::LogMessage::LogToDebug(pc::LS_VERBOSE);
#else
  rtc::LogMessage::LogToDebug(rtc::LS_NONE);
  pc::LogMessage::LogToDebug(pc::LS_NONE);
#endif

  // parse settings
  if (!setting.empty()) {
    ParseSetting(setting);
  }

  signout_ = false;

  // create signal client
  if (signal_ == nullptr) {
    signal_ = std::make_shared<pc::Signal>();

    if (signal_) {
      signal_->SetConfig(setting_.signal_uri_);
    }
  }

  LOGP_F( INFO ) << "Done";
}

PeerConnect::~PeerConnect() {
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Run() {
  rtc::ThreadManager::Instance()->CurrentThread()->Run();
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Stop() {
  rtc::ThreadManager::Instance()->CurrentThread()->Quit();
  LOGP_F( INFO ) << "Done";
}


void PeerConnect::SignIn(const std::string alias, const std::string id, const std::string password) {

  //
  // Check if already signed in
  //

  if (control_.get() != nullptr) {
    LOGP_F(WARNING) << "Already signined in.";
    return;
  }

  //
  // Initialize control
  //

  control_ = std::make_shared<pc::Control>(signal_);
  control_->RegisterObserver(this, control_);

  if (control_.get() == NULL) {
    LOGP_F(LERROR) << "Failed to create class Control.";
    return;
  }

  //
  // Initialize peer connection
  //

  if (!control_->InitializeControl()) {
    LOGP_F(LERROR) << "Failed to initialize Control.";
    control_.reset();
    return;
  }

  //
  // Set user_id and open_id
  //

  std::string user_id;
  std::string open_id;

  user_id = tolower(id);
  if (user_id == "anonymous") user_id = "";

  open_id = tolower(alias);
  if (open_id.empty()) open_id = tolower(rtc::CreateRandomUuid());

  //
  // Connect to signal server
  //

  control_->SignIn(user_id, password, open_id);
  LOGP_F( INFO ) << "Done";
  return;
}

void PeerConnect::SignOut() {
  signout_ = true;
  control_->SignOut();
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Connect(const std::string id) {
  control_->Connect(id);
  LOGP_F( INFO ) << "Done, id is " << id;
  return;
}

void PeerConnect::Disconnect(const std::string id) {
  control_->Close(id);
  LOGP_F( INFO ) << "Done, id is " << id;
  return;
}


//
// Send message to destination peer session id
//

void PeerConnect::Send(const std::string& id, const char* buffer, const size_t size) {
  control_->Send(id, buffer, size);
}

void PeerConnect::Send(const std::string& id, const char* message) {
  Send(id, message, strlen(message));
}

void PeerConnect::Send(const std::string& id, const std::string& message) {
  Send(id, message.c_str(), message.size());
}

bool PeerConnect::SyncSend(const std::string& id, const char* buffer, const size_t size) {
  return control_->SyncSend(id, buffer, size);
}

bool PeerConnect::SyncSend(const std::string& id, const char* message) {
  return SyncSend(id, message, strlen(message));
}

bool PeerConnect::SyncSend(const std::string& id, const std::string& message) {
  return SyncSend(id, message.c_str(), message.size());
}





std::string PeerConnect::CreateRandomUuid() {
  return rtc::CreateRandomUuid();
}

//
// Register Event handler
//
PeerConnect& PeerConnect::On(std::string event_id, std::function<void(PeerConnect*, std::string)> handler) {

  if (event_id.empty()) return *this;

  std::unique_ptr<EventHandler_2> f(new EventHandler_2(handler));
  event_handler_.insert(Events::value_type(event_id, std::move(f)));

  LOGP_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  return *this;
}

//
// Register Message handler
//

PeerConnect& PeerConnect::OnMessage(std::function<void(PeerConnect*, std::string, Buffer&)> handler) {
  message_handler_ = handler;
  LOGP_F( INFO ) << "A message handler has been inserted";
  return *this;
}

//
// Signal event handler
//

void PeerConnect::OnSignedIn(const std::string id) {
  signout_ = false;

  if ( event_handler_.find( "signin" ) == event_handler_.end() ) {
    return;
  }

  CallEventHandler("signin", this, id);
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::OnSignedOut(const std::string id) {
  if ( !signout_ ) {
    LOGP_F( WARNING ) << "signout_ is false, id is " << id;
    return;
  }

  if ( event_handler_.find( "signout" ) == event_handler_.end() ) {
    return;
  }

  CallEventHandler("signout", this, id);

  control_->UnregisterObserver();
  control_.reset();
  LOGP_F( INFO ) << "Done, id is " << id;
}

void PeerConnect::OnPeerConnected(const std::string id) {
  if ( event_handler_.find( "connect" ) == event_handler_.end() ) {
    return;
  }

  CallEventHandler("connect", this, id);
  LOGP_F( INFO ) << "Done, id is " << id;
}

void PeerConnect::OnPeerDisconnected(const std::string id) {
  if ( event_handler_.find( "disconnect" ) == event_handler_.end() ) {
    return;
  }

  CallEventHandler("disconnect", this, id);
  LOGP_F( INFO ) << "Done, id is " << id;
}

void PeerConnect::OnPeerMessage(const std::string id, const char* buffer, const size_t size) {
  Buffer buf(buffer, size);
  message_handler_(this, id, buf);
}

void PeerConnect::OnPeerWritable(const std::string id) {
  if ( event_handler_.find( "writable" ) == event_handler_.end() ) {
    return;
  }

  CallEventHandler("writable", this, id);
  LOGP_F( INFO ) << "Done, id is " << id;
}

void PeerConnect::OnError(const std::string id, const std::string& reason) {
  if ( event_handler_.find( "error" ) == event_handler_.end() ) {
    return;
  }

  error_reason_ = reason;
  CallEventHandler("error", this, id);
  LOGP_F( INFO ) << "Done, id is " << id << " and reason is " << reason;
}

template<typename ...A>
void PeerConnect::CallEventHandler(std::string msg_id, A&& ... args)
{
  using eventhandler_t = EventHandler_t<A...>;
  using cb_t = std::function<void(A...)>;
  const Handler_t& base = *event_handler_[msg_id];
  const cb_t& func = static_cast<const eventhandler_t&>(base).callback_;
  func(std::forward<A>(args)...);
}


bool PeerConnect::ParseSetting(const std::string& setting) {
  Json::Reader reader;
  Json::Value jsetting;

  std::string value;

  if (!reader.parse(setting, jsetting)) {
    LOGP_F(WARNING) << "Invalid setting: " << setting;
    return false;
  }

  if (rtc::GetStringFromJsonObject(jsetting, "url", &value)) {
    setting_.signal_uri_ = value;
  }

  if (rtc::GetStringFromJsonObject(jsetting, "user_id", &value)) {
    setting_.signal_id_ = value;
  }

  if (rtc::GetStringFromJsonObject(jsetting, "user_password", &value)) {
    setting_.signal_password_ = value;
  }

  return true;
}

std::string PeerConnect::tolower(const std::string& str) {
  std::locale loc;
  std::string lower_str;
  for (auto elem : str) {
    lower_str += std::tolower(elem, loc);
  }
  return lower_str;
}
