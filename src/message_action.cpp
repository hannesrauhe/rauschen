#include "message_action.hpp"
#include "server.hpp"

MessageAction::~MessageAction()
{
}

RequestPeerListAction::RequestPeerListAction( Peers& peers )
    : peers_( peers )
{
}

bool RequestPeerListAction::process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& )
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

bool PeerListAction::process( const ip_t& sender, const std::string& sender_key, const PInnerContainer& container)
{
  PPeerList list;
  list.ParseFromString(container.message());
  Logger::debug("Received peer list with "+std::to_string(list.ip_size())+ " IPs");
  for(auto i = 0; i<list.ip_size(); ++i) {
    ip_t::bytes_type ipbytes;
    for(auto j=0; j<ipbytes.size(); ++j) {
      ipbytes[j] = list.ip(i)[j];
    }
    Server::getInstance().sendPing(ip_t(ipbytes));
  }
  return true;
}
