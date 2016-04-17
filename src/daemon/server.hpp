#pragma once

#include "common.hpp"
#include "internal_commands.hpp"
#include "peers.hpp"
#include "crypto.hpp"

using asio::ip::udp;

class MessageDispatcher;

class Server
{
public:
  static Server& getInstance() {
    static Server singleton_;
    return singleton_;
  }

  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(Server const&) = delete;
  Server& operator=(Server&&) = delete;

private:
  Server();

public:
  void run();

  void stop();

  bool isRunning() {
	  return running;
  }

  void runMaintenance();

  void startReceive();

  void sendMessageTo(const PInnerContainer& msg, const std::string& receiver, ip_t single_ip = ip_t::any());

  void broadcastMessage(const PInnerContainer& msg);

  void broadcastPing(bool use_multicast = false);

  void sendPing(ip_t receiver);

  asio::io_service& getIOservice() {
    return io_service_;
  }

  Peers& getPeers() {
    return peers_;
  }
  Crypto& getCrypto() {
    return crypto_;
  }
  MessageDispatcher* getDispatcher() {
    return dispatcher_;
  }

  void sendApiResponse(const PApiResponse& resp, const asio::ip::udp::endpoint& ep);
protected:
  void sendMessageToIP( const PEncryptedContainer& message, const asio::ip::address_v6& ip );

  void pingHostsFromHostsFile();

  bool running = false;
  udp::endpoint remote_endpoint_;
  std::array<char, RAUSCHEN_MAX_PACKET_SIZE> recv_buffer_;
  asio::ip::address_v6 multicast_address_;
  asio::io_service io_service_;
  udp::socket socket_;
  Crypto crypto_;
  Peers peers_;
  MessageDispatcher* dispatcher_;
};
