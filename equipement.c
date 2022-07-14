#include<pthread.h>

#include "./common.h"

int handshake(client_socket_info_t* sock_info){
  char req[BUFF_SIZE];
  char res[BUFF_SIZE];
  int id = -1;
  message_t* req_args = init_message();
  message_t* res_args = init_message();

  req_args->id = REQ_ADD;
  encode_args(req, req_args);
  send(sock_info->server_fd, req, BUFF_SIZE, 0);
  if (read(sock_info->server_fd, res, BUFF_SIZE) != 0b00000000){
    decode_args(res, res_args);
    if(res_args->id == RES_ADD && res_args->payload_size == 2){
      id = atoi(res_args->payload);
    }
  }

  destroy_message(req_args);
  destroy_message(res_args);
  return id;
}

void* receiver(void* args) {
  char req[BUFF_SIZE];
  client_socket_info_t* sock_info = (client_socket_info_t*)args;

  while (read(sock_info->server_fd, req, BUFF_SIZE) != 0b00000000) {
    printf("< %s", req);
  }
}

void* sender(void* args){
  char req[BUFF_SIZE];
  client_socket_info_t* sock_info = (client_socket_info_t*)args;

  while (!feof(stdin)) {
    int size = read_input(req, BUFF_SIZE);

    if (size > 1) {
      send(sock_info->server_fd, req, BUFF_SIZE, 0);
    }
  }
}

int main(int argc, char const *argv[])
{
  char const* host = argv[1];
  char req[BUFF_SIZE], res[BUFF_SIZE];
  int port = atoi(argv[2]);
  client_socket_info_t* sock_info;
  pthread_t receiver_thread, sender_thread;

  sock_info = create_equipement_socket(host, port);
  int id = handshake(sock_info);
  if(id < 0){
    printf("Error\n");
    return -1;
  }

  pthread_create(&receiver_thread, NULL, receiver, (void*)sock_info);
  pthread_create(&sender_thread, NULL, sender, (void*)sock_info);

  pthread_join(sender_thread, NULL);
  
  return 0;
}
