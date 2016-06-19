#include "../../api/rauschen.h"
#include "../../api/rauschen_server.h"
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
  std::thread server_thread([]{
      rauschen_server_run();
  });
  std::cout<<"Waiting for server to come up!"<<std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(2));


  std::cout<<"Welcome to the Lunchinator"<<std::endl;
  auto msg_handle = rauschen_register_message_handler("msg");
  auto lunch_handle = rauschen_register_message_handler("lunch");

  std::string cmd;
  while ( cmd!="exit" )
  {
    std::cout<<">"<<std::endl;
    std::cin>>cmd;
    if(cmd == "send")
    {
      std::string msg;
      std::cout<<"Message to send: ";
      std::cin>>msg;
      rauschen_send_message(msg.c_str(), "msg", nullptr);
    }
    else if(cmd == "show")
    {
      auto m_h = rauschen_get_next_message( msg_handle, 0 );
      while( m_h )
      {
        std::cout<<m_h->sender<<": "<<m_h->text<<std::endl;
        m_h = rauschen_get_next_message( msg_handle, 0 );
      }
    }
  }

  rauschen_unregister_message_handler(msg_handle);
  rauschen_unregister_message_handler(lunch_handle);

  rauschen_server_stop();
  server_thread.join();
  return 0;
}
