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


#define function_tn(_x_, _y_) [](_x_, _y_)

class Throughnet {
 public:
  typedef std::map<std::string, std::string> EventData;
  typedef std::function<void (std::string, EventData)> EventHandler;

  Throughnet(EventHandler func);
  ~Throughnet();
  
  void Connect(std::string id, std::string setting);
  void Post(std::string id, std::string data);
  
 protected: 
  EventHandler handler_;
};

#endif // __THROUGHNET_THROUGHENT_H__
