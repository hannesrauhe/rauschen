#include "../api/rauschen_server.h"
#include <csignal>

int main(int argc, char* argv[]) {
  std::signal(SIGINT, [](int) { rauschen_server_stop(); });
  std::signal(SIGTERM, [](int) { rauschen_server_stop(); });

  rauschen_server_run();

  return 0;
}
