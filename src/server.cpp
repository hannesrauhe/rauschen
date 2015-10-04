#include "server.hpp"
#include "message_dispatcher.hpp"

#include <csignal>

Server::Server()
    : socket_( io_service_ ),
      crypto_( RAUSCHEN_KEY_FILE )
{
// Create the socket so that multiple may be bound to the same address.
  asio::ip::udp::endpoint listen_endpoint( udp::v6(), RAUSCHEN_PORT );
  socket_.open( listen_endpoint.protocol() );
  socket_.set_option( asio::ip::udp::socket::reuse_address( true ) );
  socket_.bind( listen_endpoint );

  multicast_address_ = asio::ip::address_v6::from_string( RAUSCHEN_MULTICAST_ADDR );
  if ( multicast_address_.is_multicast_site_local() )
  {
    try
    {
      socket_.set_option( asio::ip::multicast::join_group( multicast_address_, RAUSCHEN_PORT ) );
      Logger::info( std::string( "Joined Multicast Group " ) + multicast_address_.to_string() );
    }
    catch ( ... )
    {
      Logger::warn( "Cannot join multicast group - disabling." );
      multicast_address_ = ip_t::any();
    }
  }
  else
  {
    Logger::warn( "There is no valid site local multicast address defined" );
  }

  Logger::info( crypto_.getFingerprint( crypto_.getPubKey() ) );
  Logger::info( std::string( "Listening on " ) + listen_endpoint.address().to_string() );

  dispatcher_ = new MessageDispatcher(peers_);
  startReceive();
}

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

  std::signal(SIGINT, [] (int){ Server::getInstance().getIOservice().stop();});
  std::signal(SIGTERM, [] (int){ Server::getInstance().getIOservice().stop();});
  io_service_.run();
  Logger::info("Shutting down");
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
      sendMessageToIP( crypto_.createEncryptedContainer(), multicast_address_ );
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
        sendMessageToIP( crypto_.createEncryptedContainer(), host_address );
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
            sendMessageToIP( crypto_.createEncryptedContainer(), h_address.to_v6() );
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
//      try
      {
        auto sender = remote_endpoint_.address().to_v6();
        if(remote_endpoint_.address().is_loopback())
        {
          PInnerContainer container;
          container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
          startReceive();
          dispatcher_->executeCommand(container);
        }
        else
        {
          PEncryptedContainer outer_container;
          outer_container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
          startReceive();
          std::unique_ptr<PInnerContainer> container;
          if(crypto_.checkAndEncrypt(outer_container, container))
          {
            if(container==nullptr) {
              //just a ping
              sendMessageTo( PInnerContainer(), outer_container.pubkey(), sender );
            } else {
              dispatcher_->dispatch(sender, outer_container.pubkey(), *container);
            }
          } else {
            Logger::debug( "Received invalid message from " + sender.to_string() );
          }
        }
      }
//      catch (const std::exception& e)
//      {
//        Logger::error("Error when receiving message");
//        Logger::error(e.what());
//      }
    }
  } );
}
