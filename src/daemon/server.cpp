#include "server.hpp"
#include "message_dispatcher.hpp"
#include "logger.hpp"

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <functional>

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
      socket_.set_option( asio::ip::multicast::join_group( multicast_address_ ) );
      Logger::info( std::string( "Joined Multicast Group " ) + multicast_address_.to_string() );
    }
    catch ( const std::system_error& e)
    {
      Logger::warn( "Cannot join multicast group - disabling. Error was: "+std::string(e.what()) );
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
	std::thread maintenance([&, this]()
	{
		auto last = std::chrono::system_clock::now() - std::chrono::seconds(RAUSCHEN_BROADCAST_INTERVAL);
		while (running)
		{
			std::chrono::duration<double> diff = std::chrono::system_clock::now() - last;
			if (diff.count()>RAUSCHEN_BROADCAST_INTERVAL)
			{
				last = std::chrono::system_clock::now();
				runMaintenance();
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});

	io_service_.run();
	Logger::info("Shutting down");
	running = false;
	maintenance.join();
}

void Server::stop()
{
	Server::getInstance().getIOservice().stop();
}

void Server::runMaintenance()
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
  pingHostsFromHostsFile();

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
        //copy necessary - after calling startReceive class member may be overwritten:
        auto remote_endpoint = remote_endpoint_;
        auto sender_ip = remote_endpoint_.address().to_v6();
        if(remote_endpoint_.address().is_loopback() ||
            (sender_ip.is_v4_mapped() && sender_ip.to_v4().is_loopback()))
        {
          PInnerContainer container;
          container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
          startReceive();
          dispatcher_->executeCommand(container, remote_endpoint);
        }
        else
        {
          PEncryptedContainer outer_container;
          outer_container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
          startReceive();

//          if ( outer_container.has_pubkey() && crypto_.getPubKey() == outer_container.pubkey() )
//          { //record my own IP but ignore the message itself
//            Logger::debug( "Message from myself" );
//            peers_.add(sender, crypto_.getPubKey() );
//            return;
//          }

          std::unique_ptr<PInnerContainer> container;
          if(crypto_.checkAndEncrypt(outer_container, container))
          {
            if(container==nullptr) {
              //just a ping or message from a newer version
              sendMessageTo( PInnerContainer(), outer_container.pubkey(), sender_ip );
            } else {
              dispatcher_->dispatch(remote_endpoint, outer_container.pubkey(), *container);
            }
          } else {
            Logger::debug( "Received invalid message from " + sender_ip.to_string() );
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

void Server::pingHostsFromHostsFile()
{
  std::ifstream ip_f( RAUSCHEN_HOSTS_FILE );
  if ( !ip_f.good() )
  {
    Logger::debug( "Cannot open hosts file - no previously known hosts to contact." );
    return;
  }

  Logger::debug( "Contacting previously known peers" );
  std::string host;
  while ( std::getline( ip_f, host ) )
  {
    ip_t host_address;
    try
    {
      host_address = ip_t::from_string( host );
      sendMessageToIP( crypto_.createEncryptedContainer(), host_address );
    }
    catch ( ... )
    {
      try
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
      catch ( ... )
      {
        Logger::debug( "Could not resolve host name \"" + host + "\" from hosts file." );
        continue;
      }
    }
  }
}

void Server::broadcastMessage( const PInnerContainer& msg )
{
  for ( const auto& p : peers_.getAllKeys() )
  {
    sendMessageTo( msg, p );
  }
}

void Server::broadcastPing( bool use_multicast )
{
  if ( use_multicast )
  {
    sendMessageToIP( crypto_.createEncryptedContainer(), multicast_address_ );
  }
  else
  {
    auto cont = crypto_.createEncryptedContainer();
    for ( const auto& ip : peers_.getAllIPs() )
    {
      sendMessageToIP( cont, ip );
    }
  }
}

void Server::sendPing( ip_t receiver )
{
  auto cont = crypto_.createEncryptedContainer();
  sendMessageToIP( cont, receiver );
}

void Server::sendMessageTo( const PInnerContainer& msg, const std::string& receiver, ip_t single_ip )
{
  auto cont = crypto_.createEncryptedContainer( receiver, msg );
  if ( single_ip == ip_t::any() )
  {
    auto ips = peers_.getIpByPubKey( receiver );
    for ( const auto& ip : ips )
    {
      sendMessageToIP( cont, ip );
    }
  }
  else
  {
    sendMessageToIP( cont, single_ip );
  }
}

void Server::sendApiResponse(const PApiResponse& resp, const asio::ip::udp::endpoint& ep)
{
  Logger::debug("Sending Packet to application on port "+ std::to_string(ep.port()));
  socket_.async_send_to( asio::buffer( resp.SerializeAsString() ), ep,
      [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
      {
      } );
}

void Server::sendMessageToIP( const PEncryptedContainer& message, const asio::ip::address_v6& ip )
{
  Logger::debug( "Contacting " + ip.to_string() );
  socket_.async_send_to( asio::buffer( message.SerializeAsString() ), asio::ip::udp::endpoint( ip, RAUSCHEN_PORT ),
      [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
      {
      } );
}
