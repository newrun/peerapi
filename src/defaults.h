#ifndef __THROUGHNET_DEFAULTS_H__
#define __THROUGHNET_DEFAULTS_H__
#pragma once

#include <string>

#include "webrtc/base/basictypes.h"

extern const char kAudioLabel[];
extern const char kVideoLabel[];
extern const char kStreamLabel[];
extern const uint16_t kDefaultServerPort;

std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
std::string GetPeerConnectionString();
std::string GetDefaultServerName();
std::string GetPeerName();

#endif  // __THROUGHNET_DEFAULTS_H__
