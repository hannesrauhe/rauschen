#include "rauschen_server.h"
#include "daemon/server.hpp"
#include <csignal>

rauschen_status rauschen_server_run() {
  {
    std::ifstream f(RAUSCHEN_KEY_FILE);
    if (!f.good()) {
      Crypto::generate(RAUSCHEN_KEY_FILE);
    }
  }

  auto& s = Server::getInstance();

  if (s.isRunning()) {
    return RAUSCHEN_STATUS_ERR;
  }
  std::signal(SIGINT, [](int) { Server::getInstance().stop(); });
  std::signal(SIGTERM, [](int) { Server::getInstance().stop(); });

  //register pre-defined actions here:
  //auto dispatcher = s.getDispatcher();
  s.run();
  return RAUSCHEN_STATUS_OK;
}

rauschen_status rauschen_server_stop() {
  Server::getInstance().stop();
  return RAUSCHEN_STATUS_OK;
}
