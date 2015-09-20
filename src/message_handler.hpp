#pragma once

#include "server.hpp"
#include "peers.hpp"

class MessageHandler
{
public:
  MessageHandler( ip_t sender );

  void executeCommand( const PInnerContainer& container );

  void handleReceivedMessage( const PInnerContainer& container );
protected:
  Server& server_ = Server::getInstance();
  Peers& peers_ = server_.getPeers();
  Crypto& crypto_ = server_.getCrypto();
  ip_t sender_;
};
