#include "server.hpp"

int main(int argc, char* argv[]) {
  {
    std::ifstream f(RAUSCHEN_KEY_FILE);
    if(!f.good()) {
      Crypto::generate(RAUSCHEN_KEY_FILE);
    }
  }

  auto& s = Server::getInstance();
  s.run();
  return 0;
}
