#pragma once

#include "internal_commands.hpp"
#include "logger.hpp"
#include "message.pb.h"
#include "crypto.hpp"
#include "peers.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <functional>
#include <asio.hpp>

using asio::ip::udp;

class Server
{
public:
  const static uint32_t MAX_MSG_LEN = 32768;
  const static char* MULTICAST_ADDR;
  const static char* KEY_FILE;

  static Server& getInstance() {
    static Server singleton_;
    return singleton_;
  }

  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(Server const&) = delete;
  Server& operator=(Server&&) = delete;

private:
  Server()
    : socket_(io_service_), crypto_(KEY_FILE)
  {
    multicast_address_ = asio::ip::address_v6::from_string(MULTICAST_ADDR);

    // Create the socket so that multiple may be bound to the same address.
    asio::ip::udp::endpoint listen_endpoint(udp::v6(), RAUSCH_PORT);
    socket_.open(listen_endpoint.protocol());
    socket_.set_option(asio::ip::udp::socket::reuse_address(true));
    socket_.bind(listen_endpoint);

    // Join the multicast group.
    socket_.set_option(
        asio::ip::multicast::join_group(multicast_address_));

    Logger log;
    log.info(crypto_.getFingerprint(crypto_.getPubKey()));
    log.info(std::string("Listening on ")+listen_endpoint.address().to_string());
    startReceive();
  }

public:

  asio::io_service& getIOservice() {
    return io_service_;
  }

  void run() {
    std::thread maintenance([&, this]() {
      while(true) {
        if(peers_.empty()) {
          Logger::debug("Broadcast");
          this->sendMessageToIP(
              this->createEncryptedContainer(),
              this->multicast_address_
              );
        }
        std::this_thread::sleep_for(std::chrono::seconds(60));
      }
    });
    io_service_.run();
  }

  void startReceive();

  bool checkAndEncrypt(const PEncryptedContainer& outer, PInnerContainer& container, ip_t sender);


  PEncryptedContainer createEncryptedContainer( const std::string& receiver = "", const PInnerContainer& inner_cont = PInnerContainer() )
  {
    PEncryptedContainer enc_cont;
    enc_cont.set_version( RAUSCH_MESSAGE_FORMAT_VERSION );
    enc_cont.set_pubkey(crypto_.getPubKey());
    if ( !receiver.empty() )
    {
      PSignedContainer sign_cont;
      auto inner_cont_ptr = sign_cont.mutable_inner_cont();
      *inner_cont_ptr = inner_cont;
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
      this->sendMessageToIP(
          this->createEncryptedContainer(),
          this->multicast_address_
          );
    } else {
      auto cont = this->createEncryptedContainer();
      for(const auto& ip : peers_.getAllIPs()) {
        this->sendMessageToIP( cont, ip );
      }
    }
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
    socket_.async_send_to( asio::buffer( message.SerializeAsString() ), asio::ip::udp::endpoint( ip, RAUSCH_PORT ),
        [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
        } );
  }

  udp::endpoint remote_endpoint_;
  std::array<char, MAX_MSG_LEN> recv_buffer_;
  asio::ip::address_v6 multicast_address_;
  asio::io_service io_service_;
  udp::socket socket_;
  Crypto crypto_;
  Peers peers_;
};
