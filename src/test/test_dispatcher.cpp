#include <gtest/gtest.h>

#include "../common.hpp"
#include "../message_dispatcher.hpp"

TEST(ActionTest, Execute) {
  PInnerContainer cont;
  cont.set_type("text");
  cont.set_message("test message");
  ExecuteAction action("notify-send");
  action.process(ip_t::any(), "some key", cont);
}

TEST(DispatcherTest, General) {
//  auto dispatcher = Server::getInstance().getDispatcher();
//  dispatcher->registerNewType("text", new ExecuteAction("notify-send"));
//  PInnerContainer cont;
//  cont.set_type("text");
//  cont.set_message("test message");
//  dispatcher->dispatch(ip_t::any(), "some key", cont);
}
