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


// ========================= messages ========================= //

#define REQ_ADD   1
#define REQ_REM   2
#define RES_ADD   3
#define RES_LIST  4
#define REQ_INF   5
#define RES_INF   6
#define ERROR     7
#define OK        8


// ========================== types ========================== //

typedef int equipment_t;

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

message_t* init_message();
void set_payload(message_t* message, void* payload, size_t payload_size);
void destroy_message(message_t* message);

int read_input(char* buff, int size);
void decode_args(char* command, message_t* args);
void encode_args(char* command, message_t* args);

server_socket_info_t* create_server_socket(int port);
socket_t connect_client(server_socket_info_t* sock_info);
client_socket_info_t* create_equipment_socket(char const* host, int port);
