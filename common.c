#include"./common.h"

server_socket_info_t* create_server_socket(int port) {
  socket_t sockfd;
  sockaddr_in_t *addr_in;
  socklen_t addr_len;
  sockaddr_t* addr;
  server_socket_info_t* sock_info;
  int domain, option = 1;

  sock_info = (server_socket_info_t*) malloc(sizeof(server_socket_info_t));
  addr_in = (sockaddr_in_t*) malloc(sizeof(sockaddr_in_t));

  domain = AF_INET;
  addr_in->sin_family = domain;
  addr_in->sin_addr.s_addr = INADDR_ANY;
  addr_in->sin_port = htons(port);
  addr_len = sizeof(addr_in);

  if ((sockfd = socket(domain, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    exit(1);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
    perror("setsockopt");
    exit(2);
  }

  addr = (sockaddr_t*)&addr_in;

  if (bind(sockfd, addr, addr_len) < 0) {
    perror("bind");
    exit(3);
  }

  sock_info->port = port;
  sock_info->addr = addr;
  sock_info->addr_len = addr_len;
  sock_info->fd = sockfd;

  return sock_info;
}

socket_t create_equipement_socket(char const* host, int port) {
  socket_t sockfd, serverfd;
  sockaddr_in_t serv_addr_in;
  socklen_t addr_len;
  sockaddr_t* serv_addr;
  int domain, option = 1;

  domain = AF_INET;
  serv_addr_in.sin_family = domain;
  serv_addr_in.sin_addr.s_addr = inet_addr(host);
  serv_addr_in.sin_port = htons(port);
  addr_len = sizeof(serv_addr_in);
  serv_addr = (sockaddr_t*)&serv_addr_in;

  if ((serverfd = socket(domain, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    return 1;
  }

  if ((sockfd = connect(serverfd, serv_addr, addr_len)) < 0) {
    close(serverfd);
    printf("Connection Failed\n");
    return -1;
  }

  if (listen(sockfd, 3) != 0) {
    perror("listen");
    exit(4);
  }

  return sockfd;
}
