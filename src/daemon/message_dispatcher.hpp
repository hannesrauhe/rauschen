#pragma once

#include "action.hpp"
#include "api_actions.hpp"
#include "server.hpp"

class MessageDispatcher {
public:
  MessageDispatcher(Peers& peers) : peers_(peers), handle_producer_(0) {
    registerNewHandler( MTYPE_REQUEST_PEER_LIST, std::make_shared<RequestPeerListAction>( peers_ ) );
    registerNewHandler( MTYPE_PEER_LIST, std::make_shared<PeerListAction>(peers_) );
    registerNewHandler( MTYPE_CMD_ADD_HOST, std::make_shared<CmdAddHostAction>() );
    registerNewHandler( MTYPE_CMD_SEND, std::make_shared<CmdSendAction>() );
    registerNewHandler( MTYPE_CMD_REGISTER_HANDLER, std::make_shared<CmdRegisterHandlerAction>() );
  }

  int registerNewHandler(const std::string& mtype, std::shared_ptr<Action> action) {
    std::lock_guard<std::mutex> lck(actions_lock_);

    int handle = handle_producer_++;
    actions_[mtype].push_back(action);
    actions_by_handle_[handle] = action;
    Logger::debug("New message type registered: "+ mtype + ", Handle: "+std::to_string(handle));
    return handle;
  }

  bool unregisterHandler(int handle) {
    std::lock_guard<std::mutex> lck(actions_lock_);

    auto action_it = actions_by_handle_.find(handle);
    if(action_it==actions_by_handle_.end()) {
      return false;
    }
    auto action = action_it->second;

    for(auto& actions_vector_pair : actions_) {
      auto& actions_vector = actions_vector_pair.second;
      for(auto it = actions_vector.begin(); it!= actions_vector.end(); ++it) {
        if(action == *it) {
          actions_vector.erase(it);
          actions_by_handle_.erase(action_it);
          Logger::debug("Handler unregistered: "+std::to_string(handle) );
          return true;
        }
      }
    }

    return false;
  }

  void executeCommand( const PInnerContainer& container, const asio::ip::udp::endpoint& ep )
  {
    Server& server_ = Server::getInstance();
    if ( container.IsInitialized() && container.has_type())
    {
      findAndExecuteAction(ep, "", container);
    }
    else
    {
      Logger::error( "Cannot parse local message:" );
      Logger::error( container.InitializationErrorString() );
    }
  }

  bool dispatch( const udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container ) {
    auto sender_ip = endpoint.address().to_v6();
    Server& server_ = Server::getInstance();
    Logger::info("Signed message from "+sender_ip.to_string() +"/"+Crypto::getFingerprint(sender_key)+" received: "+container.DebugString());
    if( peers_.add(sender_ip, sender_key) ) {
      PInnerContainer cont;
      cont.set_type(MTYPE_REQUEST_PEER_LIST);
      server_.sendMessageTo(cont, sender_key, sender_ip);
    }
    auto mtype = container.type();
    if(mtype.length()>=2 && mtype[0]=='_' && mtype[1]=='_') {
      //do authentication
      return false;
    }

    return findAndExecuteAction( endpoint, sender_key, container);
  }

protected:
  bool findAndExecuteAction( const udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container ) {
    std::vector< std::shared_ptr<Action> > selected_actions;
    {
      std::lock_guard<std::mutex> lck(actions_lock_);
      auto mtype = container.type();
      auto action = actions_.find(mtype);
      if(action==actions_.end()) {
        Logger::info("Unknown message type: "+container.type());
        return false;
      }
      selected_actions = action->second;
    }

    auto success = false;
    for( auto& selected_action : selected_actions) {
      if(sender_key==Server::getInstance().getCrypto().getPubKey() && !selected_action->processMyself()) {
        Logger::debug("Will not process message because it came from myself");
        continue;
      }
      success = success || selected_action->process(endpoint, sender_key, container);
    }
    return success;
  }
protected:
  Peers& peers_;

  std::unordered_map<std::string, std::vector< std::shared_ptr<Action> > > actions_;
  std::unordered_map< int, std::shared_ptr<Action> > actions_by_handle_;
  mutable std::mutex actions_lock_;
  int handle_producer_;
};
