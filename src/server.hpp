#pragma once

#include "logger.hpp"
#include "message.pb.h"
#include "crypto.hpp"
#include "peers.hpp"
#include "internal_commands.hpp"

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
  const static uint16_t REMOTE_PORT = 2442;
  const static uint16_t LOCAL_PORT = 2443;
  const static uint32_t MAX_MSG_LEN = 1024;
  const static int MESSAGE_FORMAT_VERSION = 1;
  const char* MULTICAST_ADDR = "ff32::8000:4213";
  const char* KEY_FILE = "test.key";

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
    asio::ip::udp::endpoint listen_endpoint(udp::v6(), REMOTE_PORT);
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
      if(peers_.empty()) {
        Logger::debug("Broadcast");
        this->sendMessageToIP(
            this->createEncryptedContainer(),
            this->multicast_address_
            );
      }
      std::this_thread::sleep_for(std::chrono::seconds(60));
    });
    io_service_.run();
  }

  void startReceive()
  {
    socket_.async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        std::bind(&Server::handleReceiveFrom, this,
          std::placeholders::_1,
          std::placeholders::_2));
  }

  void handleReceiveFrom(const asio::error_code& error,
      size_t bytes_recvd)
  {
    if (error)
    {
      Logger::info(error.message());
    } else {
      if(remote_endpoint_.address().is_loopback()) {
        PInnerContainer container;
        container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
        executeCommand(container);
      } else {
        PEncryptedContainer container;
        container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
        if(crypto_.getPubKey() == container.pubkey()) {
          std::cout<<"Received actual message from myself"<<std::endl;
          return;
        }
        Logger::debug("Received Message from "+Crypto::getFingerprint(container.pubkey())+" - "+remote_endpoint_.address().to_string());
      }
    }
    startReceive();
  }

  PEncryptedContainer createEncryptedContainer( const PInnerContainer& inner_cont = PInnerContainer(), const std::string& receiver = "" )
  {
    PEncryptedContainer enc_cont;
    enc_cont.set_version( MESSAGE_FORMAT_VERSION );
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

  void executeCommand(const PInnerContainer& container) {
    if(container.IsInitialized()) {
//      std::cout<<container.command_arg()<<": "<<container.type()<<" "<<container.message()<<std::endl;
      if(container.type()==SEND_CMD) {
        if(container.has_message()) {
          PSendCommand nested_cont;
          nested_cont.ParseFromString(container.message());
          if(container.has_receiver()) {
            sendMessageTo(nested_cont.cont_to_send(), container.receiver());
          } else {
            broadcastMessage(nested_cont.cont_to_send());
          }
        } else {
          broadcastPing();
        }
      }
    } else {
      Logger::error("Cannot parse local message:");
      Logger::error(container.InitializationErrorString());
    }
  }

  void sendMessageTo(const PInnerContainer& msg, const std::string& receiver) {
    auto ips = peers_.getIpByPubKey(receiver);
    auto cont = createEncryptedContainer(msg, receiver);
    for(const auto& ip : ips ) {
      sendMessageToIP( cont, ip );
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

protected:
  void sendMessageToIP( const PEncryptedContainer& message, const asio::ip::address_v6& ip )
  {
    socket_.async_send_to( asio::buffer( message.SerializeAsString() ), asio::ip::udp::endpoint( ip, REMOTE_PORT ),
        [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
        } );
  }

  asio::ip::address_v6 multicast_address_;
  asio::io_service io_service_;
  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  std::array<char, MAX_MSG_LEN> recv_buffer_;
  Crypto crypto_;
  Peers peers_;
};
