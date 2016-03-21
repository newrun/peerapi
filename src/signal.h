/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_SIGNAL_H__
#define __THROUGHNET_SIGNAL_H__

#include <string>
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/sigslot.h"


namespace tn {

class SignalInterface
    : public rtc::RefCountInterface {
public:
  virtual bool Connect() = 0;

  // sigslots
  sigslot::signal1<std::string*> SignalOnConnected;

};

class Signal
    : public SignalInterface {
public:
  virtual bool Connect() { return false; }

private:

}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__