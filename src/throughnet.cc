/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "throughnet.h"
#include "control.h"

#include <string>
#include <locale>

Throughnet::Throughnet()
   : Throughnet(""){
}

Throughnet::Throughnet(const std::string setting) {
  // Log level
#if DEBUG || _DEBUG
  rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
#else
  rtc::LogMessage::LogToDebug(rtc::LS_NONE);
#endif

  // parse settings
  if (!setting.empty()) {
    ParseSetting(setting);
  }

  signout_ = false;

  // create signal client
  if (signal_ == nullptr) {
    signal_ = std::make_shared<tn::Signal>();

    if (signal_) {
      signal_->SetConfig(setting_.signal_uri_);
    }
  }
}

Throughnet::~Throughnet() {
}

void Throughnet::Run() {
  rtc::ThreadManager::Instance()->CurrentThread()->Run();
}

void Throughnet::Stop() {
  rtc::ThreadManager::Instance()->CurrentThread()->Quit();
}


void Throughnet::SignIn(const std::string alias, const std::string id, const std::string password) {

  //
  // Check if already signed in
  //

  if (control_.get() != nullptr) {
    LOG(LS_ERROR) << "Already signined in.";
    return;
  }

  //
  // Initialize control
  //

  control_ = std::make_unique<tn::Control>(signal_);
  control_->RegisterObserver(this);

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Failed to create class Control.";
    return;
  }

  //
  // Initialize peer connection
  //

  if (!control_->InitializeControl()) {
    LOG(LS_ERROR) << "Failed to initialize Control.";
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
  return;
}

void Throughnet::SignOut() {
  signout_ = true;
  control_->SignOut();
}

void Throughnet::Connect(const std::string id) {
  control_->Connect(id);
  return;
}

void Throughnet::Disconnect(const std::string id) {
  control_->Disconnect(id);
  return;
}


//
// Send message to destination peer session id
//

void Throughnet::Send(const std::string& id, const char* buffer, const size_t size) {
  control_->Send(id, buffer, size);
}

void Throughnet::Send(const std::string& id, const char* message) {
  Send(id, message, strlen(message));
}

void Throughnet::Send(const std::string& id, const std::string& message) {
  Send(id, message.c_str(), message.size());
}

std::string Throughnet::CreateRandomUuid() {
  return rtc::CreateRandomUuid();
}

//
// Register Event handler
//
Throughnet& Throughnet::On(std::string event_id, std::function<void(Throughnet*, std::string)> handler) {

  if (event_id.empty()) return *this;

  std::unique_ptr<EventHandler_2> f(new EventHandler_2(handler));
  event_handler_.insert(Events::value_type(event_id, std::move(f)));

  return *this;
}

//
// Register Message handler
//

Throughnet& Throughnet::OnMessage(std::function<void(Throughnet*, std::string, Buffer&)> handler) {
  message_handler_ = handler;
  return *this;
}

//
// Signal event handler
//

void Throughnet::OnSignedIn(const std::string& id) {
  signout_ = false;

  if (event_handler_.find("signedin") == event_handler_.end()) return;
  CallEventHandler("signedin", this, id);
}

void Throughnet::OnSignedOut(const std::string& id) {
  if (signout_) {
    if (event_handler_.find("signedout") == event_handler_.end()) return;
    CallEventHandler("signedout", this, id);
  }

  control_->UnregisterObserver();
  control_.reset();
}

void Throughnet::OnPeerConnected(const std::string& id) {
  if (event_handler_.find("connected") == event_handler_.end()) return;
  CallEventHandler("connected", this, id);
}

void Throughnet::OnPeerDisconnected(const std::string& id) {
  if (event_handler_.find("disconnected") == event_handler_.end()) return;
  CallEventHandler("disconnected", this, id);
}

void Throughnet::OnPeerMessage(const std::string& id, const char* buffer, const size_t size) {
  Buffer buf(buffer, size);
  message_handler_(this, id, buf);
}



template<typename ...A>
void Throughnet::CallEventHandler(std::string msg_id, A&& ... args)
{
  using eventhandler_t = EventHandler_t<A...>;
  using cb_t = std::function<void(A...)>;
  const Handler_t& base = *event_handler_[msg_id];
  const cb_t& func = static_cast<const eventhandler_t&>(base).callback_;
  func(std::forward<A>(args)...);
}


bool Throughnet::ParseSetting(const std::string& setting) {
  Json::Reader reader;
  Json::Value jsetting;

  std::string value;

  if (!reader.parse(setting, jsetting)) {
    LOG(WARNING) << "Invalid setting: " << setting;
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

std::string Throughnet::tolower(const std::string& str) {
  std::locale loc;
  std::string lower_str;
  for (auto elem : str) {
    lower_str += std::tolower(elem, loc);
  }
  return lower_str;
}
