/** @file   rauschen.cpp
 *  @date   Oct 6, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#include "rauschen.h"
#include "../common.hpp"
#include "../internal_commands.hpp"
#include "message.pb.h"

using asio::ip::udp;

asio::io_service io_service;
static udp::endpoint rauschend_ep(asio::ip::address::from_string("127.0.0.1"), RAUSCHEN_PORT);
static udp::socket s(io_service, udp::endpoint(udp::v6(), 0));


rauschen_status rauschen_add_host(const char* hostname) {
  udp::resolver resolver(io_service);
  udp::endpoint new_host = *resolver.resolve({udp::v6(), hostname, std::to_string(RAUSCHEN_PORT)});
  auto new_address = new_host.address().to_v6().to_bytes();

  PInnerContainer cont;
  PAddHost ah_cont;
  ah_cont.set_ip(new_address.data(), new_address.size());
  cont.set_type(MTYPE_CMD_ADD_HOST);
  cont.set_message(ah_cont.SerializeAsString());

  s.send_to(asio::buffer(cont.SerializeAsString()), rauschend_ep);

  char reply[RAUSCHEN_MAX_PACKET_SIZE];
  udp::endpoint sender_endpoint;
  size_t reply_length = s.receive_from(
      asio::buffer(reply, sizeof(reply)), sender_endpoint);

  return RAUSCHEN_STATUS_OK;
}
