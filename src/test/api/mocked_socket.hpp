/** @file   MockedSocket.hpp
 *  @date   Oct 25, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#pragma once

#include <gmock/gmock.h>
#include "api/socket.hpp"

class RauschenSocketMock : public RauschenSocket {
public:
  MOCK_METHOD1(sendProto, void( const PInnerContainer& cont ));

  void simulateStatusOKReceived() {
    PApiResponse resp;
    resp.set_status(0);
    callback_(resp);
  }

protected:
  virtual void startReceiving( std::function<void( const PApiResponse& )> callback ) {
    callback_ = callback;
  }

  std::function<void( const PApiResponse& )> callback_;
};
