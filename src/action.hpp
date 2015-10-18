#pragma once

#include "common.hpp"
#include "peers.hpp"
#include "message.pb.h"

class Action {
public:
  virtual ~Action();
  virtual bool process(const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container) = 0;

  virtual bool processMyself() {  return true;  }
};

class ExecuteAction : public Action {
public:
  ExecuteAction( const std::string& executable );

  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container ) override;

protected:
  const std::string executable_path_;
};

class RegisteredHandlerAction : public Action {
public:
  RegisteredHandlerAction( const asio::ip::udp::endpoint& endpoint );

  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
      const PInnerContainer& container ) override;

protected:
  const asio::ip::udp::endpoint endpoint_;
};

class RequestPeerListAction : public Action {
public:
  RequestPeerListAction( Peers& peers );

  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container ) override;

  virtual bool processMyself() { return false;  }
protected:
  Peers& peers_;
};

class PeerListAction : public Action {
public:
  PeerListAction( Peers& peers );

  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container ) override;

  virtual bool processMyself() { return false;  }
protected:
  Peers& peers_;
};
