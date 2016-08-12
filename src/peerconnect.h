/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#ifndef __PEERCONNECT_PEERCONNECT_H__
#define __PEERCONNECT_PEERCONNECT_H__

#include <string>
#include <map>
#include <memory>
#include <functional>

#include "controlobserver.h"

#define function_pc [&]

namespace pc {
  class Control;
  class Signal;
}

class PeerConnect
    : public pc::ControlObserver {
public:

  struct Setting {
    std::string signal_uri_;
    std::string signal_id_;
    std::string signal_password_;
  };

  class Buffer {  
  public:
    Buffer(): buf_(nullptr), size_(0) {};
    Buffer(const char* buf, const size_t size): buf_(buf), size_(size) {}
    const char* buf_;
    const size_t size_;
  };

  using Control = pc::Control;
  using Signal  = pc::Signal;
  using Data    = std::map<std::string, std::string>;

  //
  // APIs
  //

  static void Run();
  static void Stop();

  void SignIn(const std::string alias = "", const std::string id = "", const std::string password = "");
  void SignOut();
  void Connect(const std::string id);
  void Disconnect(const std::string id);
  void Send(const std::string& id, const char* buffer, const size_t size);
  void Send(const std::string& id, const char* buffer);
  void Send(const std::string& id, const std::string& message);
  bool SyncSend(const std::string& id, const char* buffer, const size_t size);
  bool SyncSend(const std::string& id, const char* buffer);
  bool SyncSend(const std::string& id, const std::string& message);
  std::string GetErrorMessage() { return error_reason_; }

  static std::string CreateRandomUuid();

  PeerConnect& On(std::string event_id, std::function<void(PeerConnect*, std::string)>);
  PeerConnect& OnMessage(std::function<void(PeerConnect*, std::string, Buffer&)>);


  //
  // Member functions
  //

  explicit PeerConnect();
  explicit PeerConnect(std::string setting);
  ~PeerConnect();


protected:
  // The base type that is stored in the collection.
  struct Handler_t {
    virtual ~Handler_t() = default;
  };

  // The derived type that represents a callback.
  template<typename ...A>
  struct EventHandler_t : public Handler_t {
    using cb = std::function<void(A...)>;
    cb callback_;
    EventHandler_t(cb p_callback) : callback_(p_callback) {}
  };

  template<typename ...A>
  void CallEventHandler(std::string msg_id, A&& ... args);

  using EventHandler_1 = EventHandler_t<PeerConnect*>;
  using EventHandler_2 = EventHandler_t<PeerConnect*, std::string>;
  using EventHandler_3 = EventHandler_t<PeerConnect*, std::string, Data&>;
  using Events = std::map<std::string, std::unique_ptr<Handler_t>>;
  using MessageHandler = std::function<void(PeerConnect*, std::string, Buffer&)>;

  //
  // ControlObserver implementation
  //

  void OnSignedIn(const std::string id);
  void OnSignedOut(const std::string id);
  void OnPeerConnected(const std::string id);
  void OnPeerDisconnected(const std::string id);
  void OnPeerMessage(const std::string id, const char* buffer, const size_t size);
  void OnPeerWritable(const std::string id);
  void OnError(const std::string id, const std::string& reason);


  bool ParseSetting(const std::string& setting);
  std::string tolower(const std::string& str);

  bool signout_;
  Setting setting_;
  Events event_handler_;
  MessageHandler message_handler_;

  std::shared_ptr<pc::Control> control_;
  std::shared_ptr<pc::Signal> signal_;

  std::string error_reason_;
};


#endif // __PEERCONNECT_PEERCONNECT_H__
