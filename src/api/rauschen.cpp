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

rauschen_status send_command_to_daemon(const std::string& cmd, const std::string& msg) {
  PInnerContainer cont;
  cont.set_type(cmd);
  cont.set_message(msg);

  s.send_to(asio::buffer(cont.SerializeAsString()), rauschend_ep);

  char reply[RAUSCHEN_MAX_PACKET_SIZE];
  udp::endpoint sender_endpoint;
  size_t reply_length = s.receive_from(
      asio::buffer(reply, sizeof(reply)), sender_endpoint);

  assert(reply_length);
  return static_cast<rauschen_status>(reply[0]);
}

extern "C" {
rauschen_status rauschen_add_host(const char* hostname) {
  udp::resolver resolver(io_service);
  udp::endpoint new_host = *resolver.resolve({udp::v6(), hostname, std::to_string(RAUSCHEN_PORT)});
  auto new_address = new_host.address().to_v6().to_bytes();

  PCmdAddHost ah_cont;
  ah_cont.set_ip(new_address.data(), new_address.size());
  return send_command_to_daemon(MTYPE_CMD_ADD_HOST, ah_cont.SerializeAsString());
}

rauschen_status rauschen_send_message(const char* message, const char* message_type, const char* receiver) {
  PCmdSend send_cont;
  if(receiver!=nullptr) {
    send_cont.set_receiver(receiver);
  }
  auto cont = send_cont.mutable_cont_to_send();
  cont->set_message(message);
  cont->set_type(message_type);
  return send_command_to_daemon(MTYPE_CMD_SEND, send_cont.SerializeAsString());
}

rauschen_handle_t* rauschen_register_message_handler( const char* message_type )
{
  return nullptr;
}

rauschen_message_t* rauschen_get_next_message( const rauschen_handle_t* handle )
{
  return nullptr;
}

rauschen_status rauschen_free_message( rauschen_message_t* message )
{
  return RAUSCHEN_STATUS_OK;
}

rauschen_status rauschen_unregister_message_handler( rauschen_handle_t* handle )
{
  return RAUSCHEN_STATUS_OK;
}

}
