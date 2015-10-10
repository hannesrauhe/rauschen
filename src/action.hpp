#pragma once

#include "common.hpp"
#include "peers.hpp"
#include "message.pb.h"

class Action {
public:
  virtual ~Action();
  virtual bool process(const ip_t& sender, const std::string& sender_key, const PInnerContainer& container) = 0;

  virtual bool processMyself() {  return true;  }
};

class ExecuteAction : public Action {
public:
  ExecuteAction( const std::string& executable );

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container );

protected:
  std::string executable_path_;
};

class RequestPeerListAction : public Action {
public:
  RequestPeerListAction( Peers& peers );

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container );

  virtual bool processMyself() { return false;  }
protected:
  Peers& peers_;
};

class PeerListAction : public Action {
public:
  PeerListAction( Peers& peers );

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container );

  virtual bool processMyself() { return false;  }
protected:
  Peers& peers_;
};

class CmdAddHostAction : public Action {
public:
  CmdAddHostAction();

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container );
};

class CmdSendAction : public Action {
public:
  CmdSendAction();

  bool process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container );
};
