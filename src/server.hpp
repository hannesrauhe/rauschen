#pragma once

#include "internal_commands.hpp"
#include "logger.hpp"
#include "message.pb.h"
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

  bool checkAndEncrypt(const PEncryptedContainer& outer, PInnerContainer& container, ip_t sender);

  PEncryptedContainer createEncryptedContainer( const std::string& receiver = "", const PInnerContainer& inner_cont = PInnerContainer() )
  {
    PEncryptedContainer enc_cont;
    enc_cont.set_version( RAUSCHEN_MESSAGE_FORMAT_VERSION );
    enc_cont.set_pubkey(crypto_.getPubKey());
    if ( !receiver.empty() )
    {
      PSignedContainer sign_cont;
      auto inner_cont_ptr = sign_cont.mutable_inner_cont();
      *inner_cont_ptr = inner_cont;
      if(!inner_cont_ptr->has_timestamp()) {
        inner_cont_ptr->set_timestamp(std::chrono::seconds(std::time(nullptr)).count());
      }
      if(!inner_cont_ptr->has_receiver()) {
        inner_cont_ptr->set_receiver(Crypto::getFingerprint(receiver));
      }
      sign_cont.set_signature( crypto_.sign( inner_cont_ptr->SerializeAsString() ) );
      enc_cont.set_container( crypto_.encrypt( sign_cont.SerializeAsString(), receiver ) );
    }
    return enc_cont;
  }

  void sendMessageTo(const PInnerContainer& msg, const std::string& receiver, ip_t single_ip = ip_t::any()) {
    auto cont = createEncryptedContainer(receiver, msg);
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
//    auto ip = peers_.getIpByPubKey(receiver);
//    auto cont = createEncryptedContainer(msg, receiver);
//    sendMessageToIP( cont, ip );
  }

  void broadcastPing(bool use_multicast = false) {
    if(use_multicast) {
      sendMessageToIP(
          createEncryptedContainer(),
          multicast_address_
          );
    } else {
      auto cont = createEncryptedContainer();
      for(const auto& ip : peers_.getAllIPs()) {
        sendMessageToIP( cont, ip );
      }
    }
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
