#include "server.hpp"

int main(int argc, char* argv[]) {
  auto& s = Server::getInstance();

  Crypto::gcryptInit();
  {
    std::ifstream f(s.KEY_FILE);
    if(!f.good()) {
      Crypto::generate(s.KEY_FILE);
    }
  }
  s.run();
  return 0;
}
