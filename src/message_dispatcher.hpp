#pragma once

#include "server.hpp"
#include "message_action.hpp"

class MessageDispatcher {
public:
  MessageDispatcher(Peers& peers,Crypto& crypto) : peers_(peers), crypto_(crypto) {
    actions_[MTYPE_REQUEST_PEERS] = new RequestPeersAction();
  }

  void registerNewType(std::string mtype, MessageAction& action);

  void executeCommand( const PInnerContainer& container )
  {
    Server& server_ = Server::getInstance();
    if ( container.IsInitialized() )
    {
      if ( container.type() == SEND_CMD )
      {
        if ( container.has_message() )
        {
          PSendCommand nested_cont;
          nested_cont.ParseFromString( container.message() );
          if ( container.has_receiver() )
          {
            server_.sendMessageTo( nested_cont.cont_to_send(), container.receiver() );
          }
          else
          {
            server_.broadcastMessage( nested_cont.cont_to_send() );
          }
        }
        else
        {
          server_.broadcastPing();
        }
      }
    }
    else
    {
      Logger::error( "Cannot parse local message:" );
      Logger::error( container.InitializationErrorString() );
    }
  }

  bool dispatch( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container ) {
    Server& server_ = Server::getInstance();
    Logger::info("Signed message from "+sender.to_string() +" received: "+ container.DebugString());
    if( peers_.add(sender, sender_key) ) {
      PInnerContainer cont;
      cont.set_type(MTYPE_REQUEST_PEERS);
      server_.sendMessageTo(cont, sender_key, sender);
    }
    return true;
  }
protected:
  Peers& peers_;
  Crypto& crypto_ ;
  std::unordered_map<std::string, MessageAction*> actions_;
};
