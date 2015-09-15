#include "message.pb.h"
#include "internal_commands.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>

int main(int argc, char* argv[])
{
  try
  {
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
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
