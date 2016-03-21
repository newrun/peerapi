/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_THROUGHENT_H__
#define __THROUGHNET_THROUGHENT_H__

#include "config.h"
#include <map>
#include <functional>

#include "control.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/sigslot.h"

#define function_tn(_x_, _y_) [](Throughnet* this_, _x_, _y_)


using tn::Control;
using tn::Signal;

class Throughnet
    : public sigslot::has_slots<> {
public:
  typedef std::map<std::string, std::string> Data;
  typedef std::function<void(Throughnet*, std::string msg_id, Data&)> EventHandler;
  typedef std::map<std::string, EventHandler> Events;

  explicit Throughnet(const std::string channel);
  explicit Throughnet(const std::string channel, const std::string setting);
  explicit Throughnet(const std::string channel, const std::string setting, rtc::scoped_refptr<Signal> signal);
  ~Throughnet();

  void Start();
  Throughnet& On(std::string msg_id, void(*handler) (Throughnet* this_, std::string msg_id, Data& data));


protected:
  void OnConnected(std::string& peer_id);

  Events events_;
  rtc::scoped_refptr<Control> control_;
};

#endif // __THROUGHNET_THROUGHENT_H__
