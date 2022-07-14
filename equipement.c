#include "./common.h"

int main(int argc, char const *argv[])
{
  char const* host = argv[1];
  int port = atoi(argv[2]);
  socket_t sock_fd;

  sock_fd = create_equipement_socket(host, port);
  
  return 0;
}
