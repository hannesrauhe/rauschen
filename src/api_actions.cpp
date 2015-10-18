/** @file   api_actions.cpp
 *  @date   Oct 17, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#include "api_actions.hpp"
#include "server.hpp"
#include "message_dispatcher.hpp"

ReturnsStatusAction::ReturnsStatusAction()
{
}

bool ReturnsStatusAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
    const PInnerContainer& container )
{
  auto& server = Server::getInstance();
  int8_t status = process(container) ? 0 : 1;
  server.sendAPIStatusResponse(status, endpoint);
  return true;
}

CmdAddHostAction::CmdAddHostAction()
{
}

bool CmdAddHostAction::process( const PInnerContainer& container )
{
  PCmdAddHost ah;
  ah.ParseFromString( container.message() );
  ip_t::bytes_type ipbytes;
  for(size_t j=0; j<ipbytes.size(); ++j) {
    ipbytes[j] = ah.ip()[j];
  }
  ip_t IP(ipbytes);
  Server::getInstance().sendPing(IP);
  return true;
}

CmdSendAction::CmdSendAction()
{
}

bool CmdSendAction::process( const PInnerContainer& container )
{
  PCmdSend s;
  s.ParseFromString( container.message() );
  if(s.has_receiver()) {
    try {
      Server::getInstance().sendMessageTo(s.cont_to_send(), s.receiver());
    } catch (...) {
      Logger::error("Message could not be send to Receiver: "+s.receiver());
      return false;
    }
  } else {
    Server::getInstance().broadcastMessage(s.cont_to_send());
  }
  return true;
}

CmdRegisterHandlerAction::CmdRegisterHandlerAction()
{
}

bool CmdRegisterHandlerAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
    const PInnerContainer& container )
{
  auto& server = Server::getInstance();
  auto dispatcher = server.getDispatcher();
  PCmdRegisterHandler s;
  s.ParseFromString( container.message() );
  auto handle = dispatcher->registerNewHandler( s.mtype(), std::make_shared<RegisteredHandlerAction>( endpoint ) );

  server.sendAPIStatusResponse(0, endpoint);
  return true;
}
