#pragma once

#include "action.hpp"
#include "server.hpp"

class MessageDispatcher {
public:
  MessageDispatcher(Peers& peers) : peers_(peers) {
    registerNewType( MTYPE_REQUEST_PEER_LIST, new RequestPeerListAction(peers_) );
    registerNewType( MTYPE_PEER_LIST, new PeerListAction(peers_) );
    registerNewType( MTYPE_CMD_ADD_HOST, new CmdAddHostAction() );
    registerNewType( MTYPE_CMD_SEND, new CmdSendAction() );
  }

  void registerNewType(const std::string& mtype, Action* action) {
    actions_[mtype].push_back(action);
  }

  void executeCommand( const PInnerContainer& container, const asio::ip::udp::endpoint& ep )
  {
    Server& server_ = Server::getInstance();
    if ( container.IsInitialized() && container.has_type())
    {
      int8_t status = findAndExecuteAction(ip_t::loopback(), "", container) ? 0 : 1;
      server_.sendAPIStatusResponse(status, ep);
    }
    else
    {
      Logger::error( "Cannot parse local message:" );
      Logger::error( container.InitializationErrorString() );
    }
  }

  bool dispatch( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container ) {
    Server& server_ = Server::getInstance();
    Logger::info("Signed message from "+sender.to_string() +"/"+Crypto::getFingerprint(sender_key)+" received: "+container.DebugString());
    if( peers_.add(sender, sender_key) ) {
      PInnerContainer cont;
      cont.set_type(MTYPE_REQUEST_PEER_LIST);
      server_.sendMessageTo(cont, sender_key, sender);
    }
    auto mtype = container.type();
    if(mtype.length()>=2 && mtype[0]=='_' && mtype[1]=='_') {
      //do authentication
      return false;
    }

    return findAndExecuteAction( sender, sender_key, container);
  }

protected:
  bool findAndExecuteAction( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container ) {
    auto mtype = container.type();
    auto action = actions_.find(mtype);
    if(action==actions_.end()) {
      Logger::info("Unknown message type: "+container.type());
      return false;
    }

    auto success = false;
    for( auto& selected_action : action->second) {
      if(sender_key==Server::getInstance().getCrypto().getPubKey() && !selected_action->processMyself()) {
        Logger::debug("Will not process message because it came from myself");
        continue;
      }
      success = success || selected_action->process(sender, sender_key, container);
    }
    return success;
  }
protected:
  Peers& peers_;
  std::unordered_map<std::string, std::vector<Action*> > actions_;
};
