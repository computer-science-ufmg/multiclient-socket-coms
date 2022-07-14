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

typedef struct message_t {
  int id;
  int origin;
  int destination;
  size_t payload_size;
  void* payload;
} message_t;

typedef struct server_socket_info {
  int port;
  sockaddr_t* addr;
  socklen_t addr_len;
  socket_t fd;
} server_socket_info_t;

typedef struct client_socket_info {
  socket_t fd;
  socket_t server_fd;
} client_socket_info_t;


int read_message(char* buff, int size);
message_t* decode_args(char* command);
char* encode_args(message_t* args);

server_socket_info_t* create_server_socket(int port);
socket_t connect_client(server_socket_info_t* sock_info);
client_socket_info_t* create_equipement_socket(char const* host, int port);
