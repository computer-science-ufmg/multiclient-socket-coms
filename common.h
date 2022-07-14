#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <regex.h>
#include <arpa/inet.h>

#define TRUE (1 == 1)
#define FALSE !TRUE

#define BUFF_SIZE 500

typedef int equipement_t;

typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr sockaddr_t;
typedef int socket_t;

typedef struct server_socket_info {
  int port;
  sockaddr_t* addr;
  socklen_t addr_len;
  socket_t fd;
} server_socket_info_t;

server_socket_info_t* create_server_socket(int port);
socket_t create_equipement_socket(char const* host, int port);
