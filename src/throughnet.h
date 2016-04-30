/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_THROUGHENT_H__
#define __THROUGHNET_THROUGHENT_H__

#include <map>
#include <functional>

#include "control.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/sigslot.h"

#define function_tn []

using tn::Control;
using tn::Signal;

class Throughnet
    : public sigslot::has_slots<> {
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

  typedef std::map<std::string, std::string> Data;
  typedef std::function<void(Throughnet*, std::string peer_id, Data&)> EventHandler;
  typedef std::function<void(Throughnet*, std::string peer_id, Buffer&)> DataHandler;
  typedef std::map<std::string, EventHandler> Events;

  explicit Throughnet();
  explicit Throughnet(const std::string setting);
  explicit Throughnet(const std::string setting, rtc::scoped_refptr<Signal> signal);
  ~Throughnet();

  void Connect(const std::string channel);
  bool Send(const std::string& peer_id, const char* buffer, const size_t size);
  bool Send(const std::string& peer_id, const char* buffer);
  bool Send(const std::string& peer_id, const std::string& message);
  bool Emit(const std::string& channel, const char* buffer, const size_t size);
  bool Emit(const std::string& channel, const char* buffer);
  bool Emit(const std::string& channel, const std::string& message);

  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Data& data));
  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string peer_id, Buffer& data));

protected:
  void OnConnected(const std::string& channel, const std::string& peer_id);
  void OnData(const std::string& channel, const std::string& peer_id, const char* buffer, const size_t size);

  bool ParseSetting(const std::string& setting);

  Setting setting_;
  Events event_handler_;
  std::map<std::string, DataHandler> data_handler_;

  rtc::scoped_refptr<Control> control_;
  rtc::scoped_refptr<Signal> signal_;
};

#endif // __THROUGHNET_THROUGHENT_H__
