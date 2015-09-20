#pragma once

#include "server.hpp"
#include "peers.hpp"

class MessageHandler
{
public:
  MessageHandler( ip_t sender );

  void executeCommand( const PInnerContainer& container );

  void handleReceivedMessage( const PEncryptedContainer& container );
protected:
  Server& server_ = Server::getInstance();
  Peers& peers_ = server_.getPeers();
  ip_t sender_;
};
