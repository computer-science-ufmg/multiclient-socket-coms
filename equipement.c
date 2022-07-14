#include<pthread.h>

#include "./common.h"

void print_error(int code){
  switch (code){
    case 1:
      printf("Equipment not found\n");
      break;
    case 2:
      printf("Source equipment not found\n");
      break;
    case 3:
      printf("Target equipment not found\n");
      break;
    case 4:
      printf("Equipment limit exceeded\n");
      break;
    default:
      printf("Unknown error\n");
      break;
  }
}

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
    else if (res_args->id == ERROR){
      print_error(atoi(res_args->payload));
    }
  }

  destroy_message(req_args);
  destroy_message(res_args);
  return id;
}

void handle_message(message_t* message){
  switch (message->id)
  {
    case RES_ADD:
      printf("Novo equipamento adicionado: %s\n", (char*)message->payload);
      break;
    case RES_LIST:
      printf("RES_LIST\n");
      break;
    case RES_INF:
      printf("RES_INF\n");
      break;
    case ERROR:
      printf("ERROR\n");
      break;
    case OK:
      printf("OK\n");
      break;
    default:
      printf("UNKNOWN\n");
      break;
  }
}

void* receiver(void* args) {
  char req[BUFF_SIZE];
  message_t* res_args = init_message();
  client_socket_info_t* sock_info = (client_socket_info_t*)args;

  while (read(sock_info->server_fd, req, BUFF_SIZE) != 0b00000000) {
    decode_args(req, res_args);
    handle_message(res_args);
  }

  destroy_message(res_args);
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
  if(id <= 0){
    return -1;
  }

  pthread_create(&receiver_thread, NULL, receiver, (void*)sock_info);
  pthread_create(&sender_thread, NULL, sender, (void*)sock_info);

  pthread_join(sender_thread, NULL);
  
  return 0;
}
