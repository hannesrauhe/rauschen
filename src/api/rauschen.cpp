/** @file   rauschen.cpp
 *  @date   Oct 6, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#include "rauschen.h"
#include "../common.hpp"
#include "../internal_commands.hpp"
#include "message.pb.h"
#include <array>
#include <iostream>
#include <thread>
#include <boost/lockfree/queue.hpp>

using asio::ip::udp;

class RauschendConnector {
public:
  RauschendConnector()
      : io_service_(),
        rauschend_ep_( asio::ip::address::from_string( "127.0.0.1" ), RAUSCHEN_PORT ),
        socket_( io_service_, udp::endpoint( udp::v6(), 0 ) )//,
//        service_thread_([this]{io_service_.run();})
  {
    std::cout<<"Starting API on port "+std::to_string(socket_.local_endpoint().port())<<std::endl;
    startReceive();
  }

  ~RauschendConnector() {
    io_service_.stop();
//    service_thread_.join();
  }

  void sendCommandToDaemon(const std::string& cmd, const std::string& msg) {
    PInnerContainer cont;
    cont.set_type(cmd);
    cont.set_message(msg);

    socket_.send_to(asio::buffer(cont.SerializeAsString()), rauschend_ep_);
  }

  rauschen_status getNextStatus()
  {
    rauschen_status ret;
    while ( !status_queue_.pop( ret ) )
    {
      io_service_.run_one();
    }
    return ret;
  }

  int getNextHandle()
  {
    int ret;
    while ( !handle_queue_.pop( ret ) )
    {
      io_service_.run_one();
    }
    return ret;
  }

  rauschen_message_t* getNextMsg()
  {
    rauschen_message_t* ret;
    while ( !message_queue_.pop( ret ) )
    {
      io_service_.run_one();
    }
    return ret;
  }

  asio::io_service& getIoService() {
    return io_service_;
  }

protected:
  void startReceive() {
    socket_.async_receive_from( asio::buffer( recv_buffer_ ), remote_endpoint_, [this](const asio::error_code& error,
          size_t bytes_recvd)
      {
        if (error)
        {
          std::cerr<<"Error while receiving message"<<std::endl;
        }
        else
        {
          PApiResponse container;
          container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
          if(container.has_status()) {
            status_queue_.push(static_cast<rauschen_status>(container.status()));
          } else if (container.has_handle()) {
            handle_queue_.push(container.handle());
          } else if (container.has_received_message()) {
            auto& p_msg = container.received_message();
            auto msg = new rauschen_message_t();
            msg->sender = cpy_c_str_from_protobuf(p_msg.sender());
            msg->type = cpy_c_str_from_protobuf(p_msg.received_cont().type());
            msg->text = cpy_c_str_from_protobuf(p_msg.received_cont().message());
            message_queue_.push(msg);
          }
          startReceive();
        }
      } );
  }

  static char* cpy_c_str_from_protobuf(const std::string& str) {
    char* value = new char[str.size()+1];
    std::strcpy(value, str.c_str());
    return value;
  }

  asio::io_service io_service_;
  const udp::endpoint rauschend_ep_;
  udp::socket socket_;
//  std::thread service_thread_;

  udp::endpoint remote_endpoint_;
  std::array<char, RAUSCHEN_MAX_PACKET_SIZE> recv_buffer_;

  boost::lockfree::queue<rauschen_status, boost::lockfree::capacity<128> > status_queue_;
  boost::lockfree::queue<int, boost::lockfree::capacity<128> > handle_queue_;
  boost::lockfree::queue<rauschen_message_t*, boost::lockfree::capacity<128> > message_queue_;
};

static RauschendConnector connector;

extern "C" {
rauschen_status rauschen_add_host(const char* hostname) {
  udp::resolver resolver(connector.getIoService());
  udp::endpoint new_host = *resolver.resolve({udp::v6(), hostname, std::to_string(RAUSCHEN_PORT)});
  auto new_address = new_host.address().to_v6().to_bytes();

  PCmdAddHost ah_cont;
  ah_cont.set_ip(new_address.data(), new_address.size());
  connector.sendCommandToDaemon(MTYPE_CMD_ADD_HOST, ah_cont.SerializeAsString());
  return connector.getNextStatus();
}

rauschen_status rauschen_send_message(const char* message, const char* message_type, const char* receiver) {
  PCmdSend send_cont;
  if(receiver!=nullptr) {
    send_cont.set_receiver(receiver);
  }
  auto cont = send_cont.mutable_cont_to_send();
  cont->set_message(message);
  cont->set_type(message_type);
  connector.sendCommandToDaemon(MTYPE_CMD_SEND, send_cont.SerializeAsString());
  return connector.getNextStatus();
}

rauschen_handle_t* rauschen_register_message_handler( const char* message_type )
{
  PCmdRegisterHandler send_cont;
  if(message_type==nullptr) {
    return nullptr;
  }
  send_cont.set_mtype(message_type);
  connector.sendCommandToDaemon(MTYPE_CMD_REGISTER_HANDLER, send_cont.SerializeAsString());

  auto r = new rauschen_handle_t();
  r->num = connector.getNextHandle();
  return r;
}

rauschen_message_t* rauschen_get_next_message( const rauschen_handle_t* handle )
{
  return connector.getNextMsg();
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
  //TODO: unregister at daemon
  if(handle) {
    delete handle;
  }
  return RAUSCHEN_STATUS_OK;
}

}
