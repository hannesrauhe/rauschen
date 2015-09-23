#include "server.hpp"
#include "message_handler.hpp"

#include <csignal>

void Server::run()
{
  running = true;
  std::thread maintenance( [&, this]()
  {
    while(running)
    {
      runMaintenance();
      std::this_thread::sleep_for(std::chrono::seconds(RAUSCHEN_BROADCAST_INTERVAL));
    }
  } );
// TODO: catch signals
//  std::signal(SIGINT, [this] (int){ io_service_.stop();});
//  std::signal(SIGTERM, [this] (int){ io_service_.stop();});
  io_service_.run();
  running = false;
  maintenance.join();
}

void Server::runMaintenance()
{
  if ( peers_.empty() )
  {
    if ( multicast_address_.is_multicast_site_local() )
    {
      Logger::debug( "Multicast" );
      sendMessageToIP( createEncryptedContainer(), multicast_address_ );
    }
    else
    {
      Logger::debug( "No site local multicast address specified - skipping multicast" );
    }
    std::ifstream ip_f( RAUSCHEN_HOSTS_FILE );
    if ( ip_f.good() )
    {
      Logger::debug( "Contacting previously known peers" );
      std::string host;
      std::getline( ip_f, host );
      ip_t host_address;
      try
      {
        host_address = ip_t::from_string( host );
        sendMessageToIP( createEncryptedContainer(), host_address );
      }
      catch ( ... )
      {
        asio::ip::udp::resolver r( io_service_ );
        asio::ip::udp::resolver::query q( host, "" );
        asio::ip::udp::resolver::iterator end; // End marker.
        auto it = r.resolve( q );
        while ( it != end )
        {
          auto h_address = it->endpoint().address();
          if ( h_address.is_v6() )
          {
            sendMessageToIP( createEncryptedContainer(), h_address.to_v6() );
          }
          it++;
        }
      }
    }
    else
    {
      Logger::debug( "Cannot open host file - no previously known hosts to contact." );
    }
  }
}

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
      try
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
      catch (const std::exception& e)
      {
        Logger::error("Error when receiving message");
        Logger::error(e.what());
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
  if ( !v_success ) Logger::debug( "Verification failed" );
  return v_success;
}
