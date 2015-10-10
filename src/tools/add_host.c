#include "../api/rauschen.h"
#include "stdio.h"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    printf( "Usage: %s <hostname>\n", argv[0]);
    return 1;
  }

  return rauschen_add_host(argv[1]);
}
