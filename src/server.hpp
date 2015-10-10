#pragma once

#include "internal_commands.hpp"
#include "logger.hpp"
#include "peers.hpp"

#include <ctime>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <functional>
#include <asio.hpp>
#include "crypto.hpp"

using asio::ip::udp;

class MessageDispatcher;

class Server
{
public:
  const static uint32_t MAX_MSG_LEN = 32768;

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

  void runMaintenance();

  void startReceive();

  void sendMessageTo(const PInnerContainer& msg, const std::string& receiver, ip_t single_ip = ip_t::any()) {
    auto cont = crypto_.createEncryptedContainer(receiver, msg);
    if(single_ip == ip_t::any()) {
      auto ips = peers_.getIpByPubKey(receiver);
      for(const auto& ip : ips ) {
        sendMessageToIP( cont, ip );
      }
    } else {
      sendMessageToIP( cont, single_ip );
    }
  }

  void broadcastMessage(const PInnerContainer& msg) {
    for(const auto& p : peers_.getAllKeys()) {
      sendMessageTo(msg, p);
    }
  }

  void broadcastPing(bool use_multicast = false) {
    if(use_multicast) {
      sendMessageToIP(
          crypto_.createEncryptedContainer(),
          multicast_address_
          );
    } else {
      auto cont = crypto_.createEncryptedContainer();
      for(const auto& ip : peers_.getAllIPs()) {
        sendMessageToIP( cont, ip );
      }
    }
  }

  void sendPing(ip_t receiver) {
    auto cont = crypto_.createEncryptedContainer();
    sendMessageToIP( cont, receiver );
  }

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

  void sendAPIStatusResponse(int8_t status, const asio::ip::udp::endpoint& ep) {
    socket_.async_send_to( asio::buffer( &status, 1 ), ep,
        [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {});
  }
protected:
  void sendMessageToIP( const PEncryptedContainer& message, const asio::ip::address_v6& ip )
  {
    Logger::debug("Contacting "+ip.to_string());
    socket_.async_send_to( asio::buffer( message.SerializeAsString() ), asio::ip::udp::endpoint( ip, RAUSCHEN_PORT ),
        [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
        } );
  }

  bool running = false;
  udp::endpoint remote_endpoint_;
  std::array<char, MAX_MSG_LEN> recv_buffer_;
  asio::ip::address_v6 multicast_address_;
  asio::io_service io_service_;
  udp::socket socket_;
  Crypto crypto_;
  Peers peers_;
  MessageDispatcher* dispatcher_;
};
