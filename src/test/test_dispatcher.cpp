#include <gtest/gtest.h>

#include "../common.hpp"
#include "../message_dispatcher.hpp"

class ActionCounter : public Action {
public:
  ActionCounter() {}

  virtual bool process(const asio::ip::udp::endpoint& endpoint, const std::string& sender_key, const PInnerContainer& container) {
    counter_++;
    return true;
  }

  int counter_ = 0;
};

TEST(DispatcherTest, General) {
  auto dispatcher = Server::getInstance().getDispatcher();
  auto action = std::make_shared< ActionCounter >();

  auto handle = dispatcher->registerNewHandler("test", action );
  PInnerContainer cont;
  cont.set_type("test");
  cont.set_message("test message");
  asio::ip::udp::endpoint ep(ip_t::any(),0);
  EXPECT_TRUE( dispatcher->dispatch(ep, "some key", cont) );
  EXPECT_TRUE( 1 == action->counter_ );

  EXPECT_TRUE( dispatcher->unregisterHandler(handle) );
  EXPECT_FALSE( dispatcher->unregisterHandler(handle) );

  EXPECT_FALSE( dispatcher->dispatch(ep, "some key", cont) );
  EXPECT_TRUE( 1 == action->counter_ );
}
