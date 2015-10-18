#include "action.hpp"

#include  <cstdlib>
#include "server.hpp"

Action::~Action()
{
}

RequestPeerListAction::RequestPeerListAction( Peers& peers )
    : peers_( peers )
{
}

bool RequestPeerListAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& )
{
  PInnerContainer answer;
  answer.set_type( MTYPE_PEER_LIST );
  PPeerList list;
  for ( auto& peerIP : peers_.getAllIPs() )
  {
    auto bytes = peerIP.to_bytes();
    list.add_ip( bytes.data(), bytes.size() );
  }
  answer.set_message( list.SerializeAsString() );
  Server::getInstance().sendMessageTo(answer, sender_key);
  return true;
}

PeerListAction::PeerListAction( Peers& peers )
    : peers_( peers )
{
}

bool PeerListAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container)
{
  PPeerList list;
  list.ParseFromString(container.message());
  Logger::debug("Received peer list with "+std::to_string(list.ip_size())+ " IPs");
  for(auto i = 0; i<list.ip_size(); ++i) {
    ip_t::bytes_type ipbytes;
    for(size_t j=0; j<ipbytes.size(); ++j) {
      ipbytes[j] = list.ip(i)[j];
    }
    ip_t IP(ipbytes);
    if(!peers_.isPeer(IP)) {
      Server::getInstance().sendPing(IP);
    }
  }
  return true;
}

ExecuteAction::ExecuteAction( const std::string& executable )
    : executable_path_( executable )
{
}

bool ExecuteAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& msg)
{
  auto cmd = executable_path_;
  cmd += " "+msg.message();
  auto ret = std::system(cmd.c_str());
  return (ret==0);
}

RegisteredHandlerAction::RegisteredHandlerAction( const asio::ip::udp::endpoint& endpoint )
    : endpoint_( endpoint )
{
}

bool RegisteredHandlerAction::process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
    const PInnerContainer& container )
{
  return false;
}
