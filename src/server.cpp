#include "server.hpp"
#include "message_handler.hpp"

void Server::startReceive()
  {
    socket_.async_receive_from( asio::buffer( recv_buffer_ ), remote_endpoint_,
        [this](const asio::error_code& error,
            size_t bytes_recvd)
        {
          if (error)
          {
            Logger::info(error.message());
          }
          else
          {
            auto sender = remote_endpoint_.address().to_v6();
            MessageHandler msg(sender);
            if(remote_endpoint_.address().is_loopback())
            {
              PInnerContainer container;
              container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
              startReceive();
              msg.executeCommand(container);
              return;
            }
            else
            {
              PEncryptedContainer container;
              container.ParseFromArray(recv_buffer_.data(), bytes_recvd);
              startReceive();
              if(!container.has_pubkey()) {
                Logger::debug("Received invalid message from "+sender.to_string());
                return;
              }
              if(crypto_.getPubKey() == container.pubkey())
              {
                std::cout<<"Received actual message from myself"<<std::endl;
                return;
              }
              msg.handleReceivedMessage(container);
              return;
            }
          }
        } );
  }
