/** @file   socket.hpp
 *  @date   Oct 25, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#pragma once
#include "../common.hpp"
#include "../internal_commands.hpp"
#include "message.pb.h"
#include <array>
#include <memory>
#include <thread>

using asio::ip::udp;

class RauschenSocket
{
public:
  RauschenSocket()
      : io_service_(),
        rauschend_ep_( asio::ip::address::from_string( "127.0.0.1" ), RAUSCHEN_PORT ),
        socket_( io_service_, udp::endpoint( udp::v6(), 0 ) )
  {

  }

  virtual ~RauschenSocket()
  {
    io_service_.stop();
    service_thread_->join();
  }

  void run( std::function<void( const PApiResponse& )> callback )
  {
    startReceiving( callback );
    service_thread_.reset( new std::thread( [this]
    { io_service_.run();} ) );
  }

  asio::io_service& getIoService()
  {
    return io_service_;
  }

  virtual void sendProto( const PInnerContainer& cont )
  {
    socket_.send_to( asio::buffer( cont.SerializeAsString() ), rauschend_ep_ );
  }

protected:
  virtual void startReceiving( std::function<void( const PApiResponse& )> callback )
  {
    socket_.async_receive_from( asio::buffer( recv_buffer_ ), remote_endpoint_,
        [this, callback](const asio::error_code& error,
            size_t bytes_recvd)
        {
          if (error)
          {
//            std::cerr<<"Error while receiving message"<<std::endl;
            ;
          }
          else
          {
            PApiResponse container;
            container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
            callback(container);
            startReceiving(callback);
          }
        } );
  }

  asio::io_service io_service_;
  const udp::endpoint rauschend_ep_;
  udp::socket socket_;
  std::unique_ptr<std::thread> service_thread_;
  udp::endpoint remote_endpoint_;
  std::array<char, RAUSCHEN_MAX_PACKET_SIZE> recv_buffer_;
};
