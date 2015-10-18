#include "server.hpp"
#include "message_dispatcher.hpp"

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
  dispatcher_ = new MessageDispatcher(peers_);
}

void Server::run()
{
}

void Server::runMaintenance()
{
}

void Server::startReceive()
{
}

void Server::pingHostsFromHostsFile()
{
}

void Server::broadcastMessage( const PInnerContainer& msg )
{
}

void Server::broadcastPing( bool use_multicast )
{
}

void Server::sendPing( ip_t receiver )
{
}

void Server::sendMessageTo( const PInnerContainer& msg, const std::string& receiver, ip_t single_ip )
{
}

void Server::sendApiResponse( const PApiResponse& resp, const asio::ip::udp::endpoint& ep )
{
}

void Server::sendMessageToIP( const PEncryptedContainer& message, const asio::ip::address_v6& ip )
{
}
