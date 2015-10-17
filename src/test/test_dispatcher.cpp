#include <gtest/gtest.h>

#include "../common.hpp"
#include "../message_dispatcher.hpp"

TEST(ActionTest, Execute) {
  PInnerContainer cont;
  cont.set_type("text");
  cont.set_message("test message");
  ExecuteAction action("notify-send");
  asio::ip::udp::endpoint ep(ip_t::any(),0);
  action.process(ep, "some key", cont);
}

TEST(DispatcherTest, General) {
//  auto dispatcher = Server::getInstance().getDispatcher();
//  dispatcher->registerNewType("text", new ExecuteAction("notify-send"));
//  PInnerContainer cont;
//  cont.set_type("text");
//  cont.set_message("test message");
//  dispatcher->dispatch(ip_t::any(), "some key", cont);
}
