#pragma once

#include "common.hpp"
#include "message.pb.h"

class MessageAction {
public:
  virtual ~MessageAction() {}
  virtual bool process(const ip_t& sender, const std::string& sender_key, const PInnerContainer& container) = 0;

};

class RequestPeersAction {
public:
  bool process(const ip_t& sender, const std::string& sender_key, const PInnerContainer& container) {
    return true;
  }
};
