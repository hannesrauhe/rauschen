#include "server.hpp"
#include "message_dispatcher.hpp"

int main(int argc, char* argv[]) {
  {
    std::ifstream f(RAUSCHEN_KEY_FILE);
    if(!f.good()) {
      Crypto::generate(RAUSCHEN_KEY_FILE);
    }
  }

  auto& s = Server::getInstance();

  //register pre-defined actions here:
  //auto dispatcher = s.getDispatcher();
  s.run();
  return 0;
}
