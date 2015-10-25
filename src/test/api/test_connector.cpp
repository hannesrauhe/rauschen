#include "api/connector.hpp"
#include "mocked_socket.hpp"

class ConnectorTest : public ::testing::Test {
protected:
  RauschendConnector<RauschenSocketMock> connector_;
  RauschenSocketMock& socketmock_ = connector_.getSocket();
};

MATCHER_P(ProtoEq, n, "")
{
  PInnerContainer lhs = arg;
  PInnerContainer rhs = n;
  return lhs.SerializeAsString()==rhs.SerializeAsString();
}

TEST_F(ConnectorTest, RecvMessage) {
  PInnerContainer expected_cont;
  expected_cont.set_type("ttype");
  expected_cont.set_message("tmsg");
  auto check_fkt = [this]() {
    socketmock_.simulateStatusOKReceived();
  };
  EXPECT_CALL(socketmock_, sendProto( ProtoEq(expected_cont) ));

  connector_.sendCommandToDaemon(expected_cont.type(), expected_cont.message());
}
