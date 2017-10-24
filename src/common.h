/*
 *  Copyright 2016 The PeerApi Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */


#ifndef __PEERAPI_COMMON_H__
#define __PEERAPI_COMMON_H__

namespace peerapi {

#define function_peer [&]

enum CloseCode {
  // Success
  CLOSE_NORMAL        = 0,

  // Failure
  CLOSE_GOING_AWAY    = 0x80000000,
  CLOSE_ABNORMAL,
  CLOSE_PROTOCOL_ERROR,
  CLOSE_SIGNAL_ERROR
};


const bool SYNC_OFF = false;
const bool SYNC_ON = true;

const bool FORCE_QUEUING_OFF = false;
const bool FORCE_QUEUING_ON = true;

} // namespace peerapi

#endif // __PEERAPI_COMMON_H__