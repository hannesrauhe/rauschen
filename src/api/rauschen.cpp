/** @file   rauschen.cpp
 *  @date   Oct 6, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#include "rauschen.h"
#include "connector.hpp"

static RauschendConnector<RauschenSocket> connector;

extern "C" {
rauschen_status rauschen_add_host(const char* hostname) {
  udp::resolver resolver(connector.getIoService());
  udp::endpoint new_host = *resolver.resolve({udp::v6(), hostname, std::to_string(RAUSCHEN_PORT)});
  auto new_address = new_host.address().to_v6().to_bytes();

  PCmdAddHost ah_cont;
  ah_cont.set_ip(new_address.data(), new_address.size());
  auto ret = connector.sendCommandToDaemon(MTYPE_CMD_ADD_HOST, ah_cont.SerializeAsString());
  return static_cast<rauschen_status>(ret.status());
}

rauschen_status rauschen_send_message(const char* message, const char* message_type, const char* receiver) {
  PCmdSend send_cont;
  if(receiver!=nullptr) {
    send_cont.set_receiver(receiver);
  }
  auto cont = send_cont.mutable_cont_to_send();
  cont->set_message(message);
  cont->set_type(message_type);
  auto ret = connector.sendCommandToDaemon(MTYPE_CMD_SEND, send_cont.SerializeAsString());
  return static_cast<rauschen_status>(ret.status());
}

rauschen_handle_t* rauschen_register_message_handler( const char* message_type )
{
  PCmdRegisterHandler send_cont;
  if(message_type==nullptr) {
    return nullptr;
  }
  send_cont.set_mtype(message_type);
  auto ret = connector.sendCommandToDaemon(MTYPE_CMD_REGISTER_HANDLER, send_cont.SerializeAsString());

  if(ret.status() == RAUSCHEN_STATUS_OK)
  {
    assert(ret.has_handle());
    auto r = new rauschen_handle_t();
    r->num = ret.handle();
    return r;
  }
  return nullptr;
}

rauschen_message_t* rauschen_get_next_message( const rauschen_handle_t* handle, int block )
{
  do {
    auto msg = connector.getNextMsg();
    if(msg!=nullptr) {
      return msg;
    }
  } while(block);

  return nullptr;
}

rauschen_status rauschen_free_message( rauschen_message_t* message )
{
  delete[] message->sender;
  delete[] message->text;
  delete[] message->type;
  delete message;
  return RAUSCHEN_STATUS_OK;
}

rauschen_status rauschen_unregister_message_handler( rauschen_handle_t* handle )
{
  rauschen_status ret;
  if(handle) {
    PCmdUnregisterHandler send_cont;
    send_cont.set_handle(handle->num);
    auto pk = connector.sendCommandToDaemon(MTYPE_CMD_UNREGISTER_HANDLER, send_cont.SerializeAsString());
    ret = static_cast<rauschen_status>(pk.status());
    if(ret==RAUSCHEN_STATUS_OK) {
      delete handle;
    }
  } else {
    ret = RAUSCHEN_STATUS_INVALID_ARG;
  }
  return ret;
}

}
