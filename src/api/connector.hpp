/** @file   connector.hpp
 *  @date   Oct 25, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#pragma once

#include "../common.hpp"
#include "../internal_commands.hpp"
#include "api/rauschen.h"
#include "socket.hpp"
#include <iostream>
#include <boost/lockfree/queue.hpp>
#include <future>

template< class SocketT>
class RauschendConnector
{
public:
  RauschendConnector()
      : socket_()
  {
    socket_.run( //std::bind(&RauschendConnector::handleRecv, this) );
        [this](const PApiResponse& cont)
        {
          handleRecv(cont);
        } );
  }

  PApiResponse sendCommandToDaemon( const std::string& cmd, const std::string& msg )
  {
    PInnerContainer cont;
    cont.set_type( cmd );
    cont.set_message( msg );

    PApiResponse ret;
    ret.set_status( static_cast<int>(RAUSCHEN_STATUS_TIMEOUT) );
    {
      std::lock_guard<std::mutex> lck(sync_response_lock_);
      awaited_result_.reset( new std::promise<PApiResponse>() );
      socket_.sendProto( cont );
      auto fut = awaited_result_->get_future();
      if( std::future_status::ready == fut.wait_for(std::chrono::duration<double>(timeout_)) ) {
        ret = fut.get();
      }
      awaited_result_.reset( new std::promise<PApiResponse>() );
    }
    return ret;
  }

  rauschen_message_t* getNextMsg()
  {
    rauschen_message_t* ret = nullptr;
    auto start = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
    while ( diff.count() < timeout_ &&
        !message_queue_.pop( ret ) )
    {
      std::this_thread::sleep_for(std::chrono::duration<double>(polling_interval_));
      diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start);
    }
    return ret;
  }

  asio::io_service& getIoService()
  {
    return socket_.getIoService();
  }

  SocketT& getSocket() {
    return socket_;
  }

protected:
  void handleRecv( const PApiResponse& container )
  {
    if ( container.has_received_message() )
    {
      auto& p_msg = container.received_message();
      auto msg = new rauschen_message_t();
      msg->sender = cpy_c_str_from_protobuf( p_msg.sender() );
      msg->type = cpy_c_str_from_protobuf( p_msg.received_cont().type() );
      msg->text = cpy_c_str_from_protobuf( p_msg.received_cont().message() );
      message_queue_.push( msg );
    }
    else
    {
      if(awaited_result_==nullptr) {
        std::cerr<<"Ignoring unexpected response - probably delayed"<<std::endl;
        return;
      }
      awaited_result_->set_value(container);
    }
  }

  static char* cpy_c_str_from_protobuf( const std::string& str )
  {
    char* value = new char[str.size() + 1];
    std::strcpy( value, str.c_str() );
    return value;
  }

  std::unique_ptr< std::promise<PApiResponse> > awaited_result_;
  std::mutex sync_response_lock_;
  SocketT socket_;
  boost::lockfree::queue<rauschen_message_t*, boost::lockfree::capacity<128> > message_queue_;

  double timeout_ = 1;
  double polling_interval_ = 0.1;
};
