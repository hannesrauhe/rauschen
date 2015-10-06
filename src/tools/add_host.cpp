#define ASIO_STANDALONE

#include "../api/rauschen.h"
#include <iostream>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: "<< argv[0] <<" host" << std::endl;
    return 1;
  }

  return rauschen_add_host(argv[1]);
}
