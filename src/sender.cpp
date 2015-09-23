#define ASIO_STANDALONE

#include "message.pb.h"
#include "internal_commands.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <asio.hpp>
#include "crypto.hpp"

using asio::ip::udp;

enum { max_length = 8192 };

void ping_echo(const std::string& host)
{
  Crypto::gcryptInit();
  asio::io_service io_service;

  udp::socket s(io_service, udp::endpoint(udp::v6(), RAUSCHEN_PORT));

  udp::resolver resolver(io_service);
  udp::endpoint endpoint = *resolver.resolve({udp::v6(), host, std::to_string(RAUSCHEN_PORT)});

  Crypto crypto("test.key2");
  PEncryptedContainer enc_cont;
  enc_cont.set_version( RAUSCHEN_MESSAGE_FORMAT_VERSION );
  enc_cont.set_pubkey(crypto.getPubKey());
  s.send_to(asio::buffer(enc_cont.SerializeAsString()), endpoint);
  std::cout<<"Send ping to "<< endpoint.address().to_string() <<std::endl;

  char reply[max_length];
  udp::endpoint sender_endpoint;
  size_t reply_length = s.receive_from(
      asio::buffer(reply, max_length), sender_endpoint);

  PEncryptedContainer rec_enc_cont;
  rec_enc_cont.ParseFromArray(reply, reply_length);
  std::string fingerprint = Crypto::getFingerprint(rec_enc_cont.pubkey());
  std::cout<<"Answer from "<<fingerprint<<" ("<<sender_endpoint.address().to_string()<<")"<<std::endl;

}

int main(int argc, char* argv[])
{
    if( argc == 2) {
      ping_echo(argv[1]);
      return 0;
    }
    if (argc < 3)
    {
      std::cerr << "Usage: sender <receiver> <message> [<type> [<relay_address>] [<port>]]" << std::endl;
      return 1;
    }

    asio::io_service io_context;
    auto ip = asio::ip::address_v6::loopback();
    auto port = 2442;
    PInnerContainer container;
    container.set_type(SEND_CMD);

    PSendCommand send;
    send.set_receiver(argv[1]);
    auto nested_container = send.mutable_cont_to_send();
    nested_container->set_message(argv[2]);
    nested_container->set_type("Text");
    if(argc>3) {
      nested_container->set_type(argv[3]);
    }
    if(argc>4) {
      ip = asio::ip::address_v6::from_string(argv[4]);
    }
    if(argc>5) {
      port = std::atoi(argv[5]);
    }

    asio::ip::udp::endpoint e(ip, port);
    asio::ip::udp::socket s(io_context, e.protocol());
    s.async_send_to(
        asio::buffer(container.SerializeAsString()), e,
        [container] (const asio::error_code& error,
            size_t bytes_recvd) {
          std::cout<<container.DebugString()<<std::endl;
    });
    io_context.run();

  return 0;
}
