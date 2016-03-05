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


#define function_tn(_x_) [](Throughnet* this_, _x_)

class Throughnet {
 public:
  typedef std::map<std::string, std::string> Data;
  typedef std::function<void(Throughnet*, Data&)> EventHandler;
  typedef std::map<std::string, EventHandler> Events;

  Throughnet(std::string id, std::string setting);

  bool Connect(std::string id, std::string setting);
  void Send(std::string id, std::string data);
  Throughnet& On(std::string name, void(*handler) (Throughnet* this_, Data& data));
  
 protected: 
  ~Throughnet();

  Events events_;
};

#endif // __THROUGHNET_THROUGHENT_H__
