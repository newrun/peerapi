/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_DUMMY_SIGNAL_H__
#define __THROUGHNET_DUMMY_SIGNAL_H__

#include <map>
#include <vector>
#include "signal.h"

namespace tn {

class DummySignal
  : public Signal {
public:

  virtual bool Connect();

private:

}; // class DummySignal

} // namespace tn


#endif // __THROUGHNET_DUMMY_SIGNAL_H__

