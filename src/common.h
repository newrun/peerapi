/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */


#ifndef __PEERCONNECT_COMMON_H__
#define __PEERCONNECT_COMMON_H__

namespace pc {

#define function_pc [&]

enum CloseCode {
  // Success
  CLOSE_NORMAL        = 0,

  // Failure
  CLOSE_GOING_AWAY,
  CLOSE_ABNORMAL,
  CLOSE_PROTOCOL_ERROR,
  CLOSE_SIGNAL_ERROR
};


const bool WAITING_OFF = false;
const bool WAITING_ON = true;

const bool FORCE_QUEUING_OFF = false;
const bool FORCE_QUEUING_ON = true;

} // namespace pc

#endif // __PEERCONNECT_COMMON_H__