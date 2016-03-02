/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_THROUGHENT_H__
#define __THROUGHNET_THROUGHENT_H__

#include <map>
#include <string>
#include <functional>


#define function_tn(_x_, _y_) [](Throughnet* this_, _x_, _y_)

class Throughnet;

namespace tn {
typedef std::map<std::string, std::string> EventData;
typedef std::function<void(Throughnet* this_, std::string, EventData&)> EventHandler;
}

class Throughnet {
 public:

//  Throughnet(tn::EventHandler& handler);
  Throughnet(void (*handler) (Throughnet* this_, std::string, tn::EventData&));
  ~Throughnet();
  
  void Connect(std::string id, std::string setting);
  void Send(std::string id, std::string data);
  
 protected: 
  tn::EventHandler handler_;
};

#endif // __THROUGHNET_THROUGHENT_H__
