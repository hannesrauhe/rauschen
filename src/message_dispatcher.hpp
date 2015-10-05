#pragma once

#include "server.hpp"
#include "message_action.hpp"

class MessageDispatcher {
public:
  MessageDispatcher(Peers& peers) : peers_(peers) {
    registerNewType( MTYPE_REQUEST_PEER_LIST, new RequestPeerListAction(peers_) );
    registerNewType( MTYPE_PEER_LIST, new PeerListAction(peers_) );
  }

  void registerNewType(const std::string& mtype, MessageAction* action) {
    actions_[mtype].push_back(action);
  }

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
    Logger::info("Signed message with from "+sender.to_string() +"/"+Crypto::getFingerprint(sender_key)+" received: "+container.DebugString());
    if( peers_.add(sender, sender_key) ) {
      PInnerContainer cont;
      cont.set_type(MTYPE_REQUEST_PEER_LIST);
      server_.sendMessageTo(cont, sender_key, sender);
    }
    auto action = actions_.find(container.type());
    if(action==actions_.end()) {
      Logger::info("Unknown message type: "+container.type());
      return false;
    }
    for( auto& selected_action : action->second) {
      selected_action->process(sender, sender_key, container);
    }
    return true;
  }
protected:
  Peers& peers_;
  std::unordered_map<std::string, std::vector<MessageAction*> > actions_;
};
