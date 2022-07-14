#include<string.h>

#include"./common.h"

char* slice(char* str, int start, int end) {
  int i, size = end - start;
  char* res = (char*)malloc(sizeof(char) * size);
  strncmp(res, str + start, size);
  res[size] = '\0';
  return res;
}

int read_message(char* buff, int size) {
  char c;
  int pos = 0;
  printf("> ");
  while (pos < (size - 1) && !feof(stdin) && (c = getchar())) {
    if (c == '\n' || c == '\0') {
      break;
    }
    buff[pos] = c;
    pos++;
  }
  buff[pos] = '\n';
  pos++;
  return pos;
}

void command_to_str(char* command) {
  char* char_pos = strchr(command, '\n');
  *char_pos = '\0';
}

void str_to_command(char* command) {
  char* char_pos = strchr(command, '\0');
  *char_pos = '\n';
}

message_t* decode_args(char* command) {
  message_t* args = (message_t*) malloc(sizeof(message_t));
  command_to_str(command);

  char* id = slice(command, 0, 2);
  char* origin = slice(command, 3, 5);
  char* destination = slice(command, 6, 8);
  int payload_size = strlen(command) - 9;
  char* payload = slice(command, 9, strlen(command));

  args->id = atoi(id);
  args->origin = atoi(origin);
  args->destination = atoi(destination);
  args->payload = payload;
  args->payload_size = (void*)payload_size;

  free(id);
  free(origin);
  free(destination);

  return args;
}

char* encode_args(message_t* args) {
  char* command;
  char* payload = (char*)malloc(sizeof(char) * args->payload_size);
  memcpy(payload, args->payload, args->payload_size);

  sprintf(command, "%02d%02d%02d%s", args->id, args->origin, args->destination, args->payload);

  free(payload);
  return command;
}

sockaddr_in_t* get_local_addr_in(int port){
  sockaddr_in_t* addr_in = (sockaddr_in_t*)malloc(sizeof(sockaddr_in_t));

  addr_in->sin_family = AF_INET;
  addr_in->sin_addr.s_addr = INADDR_ANY;
  addr_in->sin_port = htons(port);
  return addr_in;
}

server_socket_info_t* create_server_socket(int port) {
  socket_t sockfd;
  sockaddr_in_t *addr_in;
  socklen_t addr_len;
  sockaddr_t* addr;
  server_socket_info_t* sock_info;
  int option = 1;

  sock_info = (server_socket_info_t*) malloc(sizeof(server_socket_info_t));
  addr_in = get_local_addr_in(port);
  addr_len = sizeof(sockaddr_in_t);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0) {
    perror("setsockopt");
    exit(2);
  }

  addr = (sockaddr_t*)addr_in;

  if (bind(sockfd, addr, addr_len) < 0) {
    perror("bind");
    exit(3);
  }

  if (listen(sockfd, 3) < 0) {
    perror("listen");
    exit(4);
  }

  sock_info->port = port;
  sock_info->addr = addr;
  sock_info->addr_len = addr_len;
  sock_info->fd = sockfd;

  return sock_info;
}

socket_t connect_client(server_socket_info_t* sock_info) {
  socket_t client_fd;
  socket_t server_sock = sock_info->fd;
  sockaddr_t* addr = sock_info->addr;
  socklen_t addr_len = sizeof(*addr);
  
  if ((client_fd = accept(server_sock, addr, &addr_len)) < 0) {
    perror("accept");
    exit(5);
  }
  return client_fd;
}

client_socket_info_t* create_equipement_socket(char const* host, int port) {
  socket_t sockfd, serverfd;
  sockaddr_in_t serv_addr_in;
  socklen_t addr_len;
  sockaddr_t* serv_addr;
  client_socket_info_t* sock_info = (client_socket_info_t*) malloc(sizeof(client_socket_info_t));
  int domain, option = 1;

  domain = AF_INET;
  serv_addr_in.sin_family = domain;
  serv_addr_in.sin_addr.s_addr = inet_addr(host);
  serv_addr_in.sin_port = htons(port);
  addr_len = sizeof(serv_addr_in);
  serv_addr = (sockaddr_t*)&serv_addr_in;

  if ((serverfd = socket(domain, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    exit(1);
  }

  if ((sockfd = connect(serverfd, serv_addr, addr_len)) < 0) {
    perror("connect");
    close(serverfd);
    exit(2);
  }

  sock_info->fd = sockfd;
  sock_info->server_fd = serverfd;
  return sock_info;
}
