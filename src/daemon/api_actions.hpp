/** @file   api_actions.hpp
 *  @date   Oct 17, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#pragma once

#include "action.hpp"

class ReturnsStatusAction : public Action {
public:
  ReturnsStatusAction();

  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
      const PInnerContainer& container ) override;

protected:
  virtual bool process( const PInnerContainer& container ) = 0;
};

class CmdAddHostAction : public ReturnsStatusAction {
public:
  CmdAddHostAction();

protected:
  bool process( const PInnerContainer& container ) override;
};

class CmdSendAction : public ReturnsStatusAction {
public:
  CmdSendAction();

protected:
  bool process( const PInnerContainer& container ) override;
};

class CmdRegisterHandlerAction : public Action {
public:
  CmdRegisterHandlerAction();

protected:
  bool process( const asio::ip::udp::endpoint& endpoint, const std::string& sender_key,
      const PInnerContainer& container ) override;
};

class CmdUnregisterHandlerAction : public ReturnsStatusAction {
public:
  CmdUnregisterHandlerAction();

protected:
  bool process( const PInnerContainer& container ) override;
};
