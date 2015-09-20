#include "server.hpp"
#include "message_handler.hpp"

void Server::startReceive()
{
  socket_.async_receive_from( asio::buffer( recv_buffer_ ), remote_endpoint_, [this](const asio::error_code& error,
      size_t bytes_recvd)
  {
    if (error)
    {
      Logger::info(error.message());
    }
    else
    {
      auto sender = remote_endpoint_.address().to_v6();
      MessageHandler msg(sender);
      if(remote_endpoint_.address().is_loopback())
      {
        PInnerContainer container;
        container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
        startReceive();
        msg.executeCommand(container);
      }
      else
      {
        PEncryptedContainer outer_container;
        outer_container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
        startReceive();
        PInnerContainer container;
        if(checkAndEncrypt(outer_container, container, sender))
        {
          msg.handleReceivedMessage(container);
        }
      }
    }
  } );
}

bool Server::checkAndEncrypt( const PEncryptedContainer& outer, PInnerContainer& inner_container, ip_t sender )
{
  Logger::debug( "Received Message from " + Crypto::getFingerprint( outer.pubkey() ) + " - " + sender.to_string() );
  if ( !outer.has_pubkey() )
  {
    Logger::debug( "Received invalid message from " + sender.to_string() );
    return false;
  }
  if ( crypto_.getPubKey() == outer.pubkey() )
  {
    Logger::debug( "Message came from myself" );
    //record ip
    return false;
  }
  if ( !outer.has_container() )
  {
    Logger::debug( "Message was a ping" );
    //just a ping
    sendMessageTo( PInnerContainer(), outer.pubkey(), sender );
    return false;
  }
  PSignedContainer sig_cont;
  sig_cont.ParseFromString( crypto_.decrypt( outer.container() ) );
  inner_container.CopyFrom( sig_cont.inner_cont() );
  bool v_success = crypto_.verify( inner_container.SerializeAsString(), sig_cont.signature(), outer.pubkey() );
  if ( !v_success ) Logger::debug( "Verfication failed" );
  return v_success;
}
