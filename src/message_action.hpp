#pragma once

#include "common.hpp"
#include "peers.hpp"
#include "message.pb.h"

class MessageAction {
public:
  virtual ~MessageAction();
  virtual bool process(const ip_t& sender, const std::string& sender_key, const PInnerContainer& container) = 0;

};

class RequestPeerListAction : public MessageAction {
public:
  RequestPeerListAction( Peers& peers );

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& );

protected:
  Peers& peers_;
};

class PeerListAction : public MessageAction {
public:
  PeerListAction( Peers& peers );

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& );

protected:
  Peers& peers_;
};
