#include "server.hpp"

const char* Server::MULTICAST_ADDR = "ff32::8000:4213";
const char* Server::KEY_FILE = "test.key";

int main(int argc, char* argv[]) {
  Crypto::gcryptInit();
  {
    std::ifstream f(Server::KEY_FILE);
    if(!f.good()) {
      Crypto::generate(Server::KEY_FILE);
    }
  }

  auto& s = Server::getInstance();
  s.run();
  return 0;
}
