/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "throughnet.h"
#include "control.h"


Throughnet::Throughnet()
   : Throughnet("", "", nullptr){
}

Throughnet::Throughnet(const std::string id)
  : Throughnet(id, "", nullptr) {
}

Throughnet::Throughnet(const std::string id, const std::string setting)
   : Throughnet(id, setting, nullptr) {
}

Throughnet::Throughnet(const std::string id, const std::string setting, std::shared_ptr<Signal> signal)
   : signal_(signal), id_(id)
{
  // Default settings
  setting_.signal_uri_ = "wss://signal.throughnet.com/hello";

  // parse settings
  if (!setting.empty()) {
    ParseSetting(setting);
  }

  if (id_.empty()) {
    id_ = rtc::CreateRandomUuid();
  }

  // create signal client
  if (signal == nullptr) {
    signal_ = std::make_shared<tn::Signal>();

    if (signal_) {
      signal_->SetConfig(setting_.signal_uri_,
        setting_.signal_id_,
        setting_.signal_password_);
    }
  }
}

Throughnet::~Throughnet() {
}

void Throughnet::Run() {
  rtc::ThreadManager::Instance()->CurrentThread()->Run();
}


void Throughnet::GetReady() {

  //
  // Initialize control
  //

  control_ = std::make_unique<tn::Control>(this, id_, signal_);

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Failed to create class Control.";
    return;
  }

  //
  // Initialize peer connection
  //

  if (!control_->InitializeControl()) {
    LOG(LS_ERROR) << "Failed to initialize Control.";
    return;
  }

  //
  // Connect to signal server
  //

  control_->SignIn();
  return;

}

void Throughnet::Connect(const std::string id) {

  signal_->JoinChannel(id);
  return;
}


//
// Send message to destination peer session id
//

void Throughnet::Send(const std::string& peer_id, const char* buffer, const size_t size) {
  control_->Send(buffer, size, peer_id);
}

void Throughnet::Send(const std::string& peer_id, const char* message) {
  Send(peer_id, message, strlen(message));
}

void Throughnet::Send(const std::string& peer_id, const std::string& message) {
  Send(peer_id, message.c_str(), message.size());
}


//
// Register Event handler
//
Throughnet& Throughnet::On(std::string msg_id, std::function<void(Throughnet*, std::string)> handler) {

  if (msg_id.empty()) return *this;

  std::unique_ptr<EventHandler_2> f(new EventHandler_2(handler));
  event_handler_.insert(Events::value_type(msg_id, std::move(f)));

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

void Throughnet::OnPeerConnected(const std::string& id) {
  if (event_handler_.find("connected") == event_handler_.end()) return;
  CallEventHandler("connected", this, id);
}

void Throughnet::OnReady(const std::string& id) {
  if (event_handler_.find("ready") == event_handler_.end()) return;
  CallEventHandler("ready", this, id);
}

//
// Signal data handler
//

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
