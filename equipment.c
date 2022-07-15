#include <pthread.h>
#include <string.h>

#include "./common.h"

int id;
int equipments[MAX_CLIENTS];
int equiments_size = 0;

void init_equipments() {
  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    equipments[i] = -1;
  }
}

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

void print_ok(int code) {
  switch (code) {
  case 1:
    printf("Successful removal\n");
    break;
  default:
    printf("Unknown message\n");
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

// ======================= Handlers ======================= //

int handle_req_rem(message_t* request) {
  int origin = request->origin;
  if(origin < 0){
    return 1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++){
    if (equipments[i] == origin){
      equipments[i] = -1;
      equiments_size--;
      printf("Equipment %02d removed\n", origin);
      return 0;
    }
  }

  return 1;
  
}

int handle_res_add(message_t* message) {
  int id = atoi(message->payload);

  if(equiments_size >= MAX_CLIENTS){
    return 1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (equipments[i] == -1) {
      equipments[i] = id;
      equiments_size++;
      printf("Equipment %02d added\n", id);
      return 0;
    }
  }
  
  return 0;
}

int handle_res_list(message_t* message){
  equiments_size = (int)(message->payload_size / 2);
  char* id_str;

  if (equiments_size > MAX_CLIENTS) {
    return 1;
  }

  // printf("Equipment list:\n");
  for(int i = 0; i < equiments_size; i++){
    id_str = slice(message->payload, i * 2, i * 2 + 2);
    equipments[i] = atoi(id_str);
    // printf("%02d\n", equipments[i]);
    free(id_str);
  }

  for(int i = equiments_size; i < MAX_CLIENTS; i++){
    equipments[i] = -1;
  }

  return 0;
}

int handle_res_inf(message_t* message){
  printf("RES_INF\n");
  return 0;
}

int handle_error(message_t* message){
  if (message->payload_size == 2) {
    int code = atoi(message->payload);
    print_error(code);
    if (code == 1) {
      return -1;
    }
  }
  return 0;
}

int handle_ok(message_t* message){
  if (message->payload_size == 2) {
    int code = atoi(message->payload);
    print_ok(code);
    if (code == 1) {
      return -1;
    }
  }
  return 0;
}

int handle_message(message_t* message){
  switch (message->id)
  {
    case REQ_REM:
      return handle_req_rem(message);

    case RES_ADD:
      return handle_res_add(message);

    case RES_LIST:
      return handle_res_list(message);

    case RES_INF:
      return handle_res_inf(message);
      
    case ERROR:
      return handle_error(message);      

    case OK:
      return handle_ok(message);
      
    default:
      printf("UNKNOWN\n");
      return 0;
  }
}

// ======================= Lifecycle ======================= //

void disconnect(client_socket_info_t* sock_info){
  char req[BUFF_SIZE], res[BUFF_SIZE];
  message_t* req_args = init_message();

  req_args->id = REQ_REM;
  req_args->origin = id;
  encode_args(req, req_args);
  send(sock_info->server_fd, req, BUFF_SIZE, 0);

  destroy_message(req_args);
}

int receive_initial_list(client_socket_info_t* sock_info) {
  char res[BUFF_SIZE];

  if (read(sock_info->server_fd, res, BUFF_SIZE) != 0b00000000) {
    message_t* res_args = init_message();
    decode_args(res, res_args);
    if (res_args->id != RES_LIST) {
      destroy_message(res_args);
      return -1;
    }
    if(handle_message(res_args) == -1){
      return -1;
    }
    destroy_message(res_args);
  }

  return 0;
}

// ======================= Threads ======================= //

void* receiver(void* args) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();
  client_socket_info_t* sock_info = (client_socket_info_t*)args;

  while (read(sock_info->server_fd, res, BUFF_SIZE) != 0b00000000) {
    decode_args(res, res_args);
    int code = handle_message(res_args);
    if(code == -1){
      break;
    }
    else if(code == 1){
      printf("Error handling response\n");
    }
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

  disconnect(sock_info);
}

// ======================= Main ======================= //

int main(int argc, char const *argv[])
{
  char const* host = argv[1];
  int port = atoi(argv[2]);
  char req[BUFF_SIZE], res[BUFF_SIZE];
  client_socket_info_t* sock_info;
  pthread_t receiver_thread, sender_thread;

  init_equipments();

  sock_info = create_equipment_socket(host, port);
  id = handshake(sock_info);
  if(id <= 0){
    return -1;
  }
  printf("New ID: %d\n", id);

  if(receive_initial_list(sock_info) < 0){
    return -1;
  }

  pthread_create(&receiver_thread, NULL, receiver, (void*)sock_info);
  pthread_create(&sender_thread, NULL, sender, (void*)sock_info);

  pthread_join(sender_thread, NULL);
  pthread_join(receiver_thread, NULL);
  
  return 0;
}
