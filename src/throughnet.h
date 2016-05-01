/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_THROUGHENT_H__
#define __THROUGHNET_THROUGHENT_H__

#include <map>
#include <memory>
#include <functional>

#include "controlobserver.h"

#define function_tn []

namespace tn {
  class Control;
  class Signal;
}

class Throughnet
    : public tn::ControlObserver {
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

  using Control = tn::Control;
  using Signal  = tn::Signal;
  using Data    = std::map<std::string, std::string>;

  explicit Throughnet();
  explicit Throughnet(const std::string setting);
  explicit Throughnet(const std::string setting, std::shared_ptr<tn::Signal> signal);
  ~Throughnet();

  static void Throughnet::Run();

  void Connect(const std::string channel);
  bool Send(const std::string& peer_id, const char* buffer, const size_t size);
  bool Send(const std::string& peer_id, const char* buffer);
  bool Send(const std::string& peer_id, const std::string& message);
  bool Emit(const std::string& channel, const char* buffer, const size_t size);
  bool Emit(const std::string& channel, const char* buffer);
  bool Emit(const std::string& channel, const std::string& message);

  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id));
  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Data& data));
  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Buffer& data));

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

  using EventHandler_1 = EventHandler_t<Throughnet*>;
  using EventHandler_2 = EventHandler_t<Throughnet*, std::string>;
  using EventHandler_3 = EventHandler_t<Throughnet*, std::string, Data&>;
  using EventHandler_OnData = EventHandler_t<Throughnet*, std::string, Buffer&>;
  using Events = std::map<std::string, std::unique_ptr<Handler_t>>;


  void OnConnected(const std::string& channel, const std::string& peer_id);
  void OnData(const std::string& channel, const std::string& peer_id, const char* buffer, const size_t size);

  bool ParseSetting(const std::string& setting);

  Setting setting_;
  Events event_handler_;

  std::unique_ptr<tn::Control> control_;
  std::shared_ptr<tn::Signal> signal_;
};


#endif // __THROUGHNET_THROUGHENT_H__
