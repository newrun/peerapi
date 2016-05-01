/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */
 
#include "throughnet.h"


Throughnet::Throughnet()
   : Throughnet("", nullptr){
}

Throughnet::Throughnet(const std::string setting)
   : Throughnet(setting, nullptr) {
}

Throughnet::Throughnet(const std::string setting, rtc::scoped_refptr<Signal> signal)
   : signal_(signal)
{
  // Default settings
  setting_.signal_uri_ = "wss://signal.throughnet.com/hello";

  // parse settings
  if (!setting.empty()) {
    ParseSetting(setting);
  }

  // create signal client
  if (signal == nullptr) {
    signal_ = new rtc::RefCountedObject<tn::Signal>();

    if (signal_) {
      signal_->SetConfig(setting_.signal_uri_,
        setting_.signal_id_,
        setting_.signal_password_);
    }
  }
}

Throughnet::~Throughnet() {
}


void Throughnet::Connect(const std::string channel) {

  //
  // Initialize control
  //

  control_ = new rtc::RefCountedObject<Control>(channel, signal_);

  if (control_.get() == NULL) {
    LOG(LS_ERROR) << "Failed to create class Control.";
    return;
  }

  //
  // connect sigslot
  //

  control_->SignalOnConnected_.connect(this, &Throughnet::OnConnected);
  control_->SignalOnData_.connect(this, &Throughnet::OnData);

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


//
// Send message to destination peer session id
//

bool Throughnet::Send(const std::string& peer_id, const char* buffer, const size_t size) {
  return control_->Send(buffer, size);
}

bool Throughnet::Send(const std::string& peer_id, const char* message) {
  return Send(peer_id, message, strlen(message));
}

bool Throughnet::Send(const std::string& peer_id, const std::string& message) {
  return Send(peer_id, message.c_str(), message.size());
}

//
// Emit message to all peer in the channel
//

bool Throughnet::Emit(const std::string& channel, const char* buffer, const size_t size) {
  if (control_->channel_name() != channel)
    return false;

  return control_->Send(buffer, size);
}

bool Throughnet::Emit(const std::string& channel, const char* message) {
  return Emit(channel, message, strlen(message));
}

bool Throughnet::Emit(const std::string& channel, const std::string& message) {
  return Emit(channel, message.c_str(), message.size());
}


//
// Register Event handler
//
Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id)) {

  std::unique_ptr<EventHandler_2> f(new EventHandler_2(handler));

  if (msg_id == "connected") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }
  else if (msg_id == "disconnected") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }
  else if (msg_id == "signin") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }
  else if (msg_id == "signout") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }

  return *this;
}

Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Data& data)) {
  std::unique_ptr<EventHandler_3> f(new EventHandler_3(handler));

  if (msg_id == "disconnected") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }
  else if (msg_id == "error") {
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }

  return *this;
}

//
// Register OnData event handler
//
Throughnet& Throughnet::On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Buffer& data)) {

  if (msg_id.length() > 0) {
    std::unique_ptr<EventHandler_OnData> f(new EventHandler_OnData(handler));
    event_handler_.insert(Events::value_type(msg_id, std::move(f)));
  }

  return *this;
}

//
// Signal event handler
//

void Throughnet::OnConnected(const std::string& channel, const std::string& peer_id) {
  if (event_handler_.find("connected") == event_handler_.end()) return;
  CallEventHandler("connected", this, peer_id);
}

//
// Signal data handler
//

void Throughnet::OnData(const std::string& channel, const std::string& peer_id, const char* buffer, const size_t size) {
  if (event_handler_.find(channel) == event_handler_.end()) return;
  Buffer buf(buffer, size);
  CallEventHandler(channel, this, peer_id, buf);
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
