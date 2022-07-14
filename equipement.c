#include "./common.h"

int main(int argc, char const *argv[])
{
  char const* host = argv[1];
  char req[BUFF_SIZE], res[BUFF_SIZE];
  int port = atoi(argv[2]);
  client_socket_info_t* sock_info;

  sock_info = create_equipement_socket(host, port);

  while (!feof(stdin) && strncmp(req, "kill", 5)) {
    int size = read_message(req, BUFF_SIZE);

    if (size > 1) {
      send(sock_info->server_fd, req, BUFF_SIZE, 0);
      // terminate_command_string(req);

      // read(serverfd, res, buffsize);
      // terminate_command_string(res);
      // if (strlen(res) > 1) {
      //   printf("< %s\n", res);
      // }
    }
  }
  
  return 0;
}
