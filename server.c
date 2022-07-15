#include <pthread.h>
#include <stdio.h>

#include"./common.h"

// ======================= Types ======================= //

typedef struct worker_args {
  int index;
  socket_t client_fd;
} worker_args_t;

// ======================= Globals ======================= //

pthread_t threads[MAX_CLIENTS];
worker_args_t* connections[MAX_CLIENTS];
int clients = 0;
pthread_mutex_t mutex;

// ======================= Utils ======================= //

int get_available_thread_index() {
  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (threads[i] == 0) {
      return i;
    }
  }
  return -1;
}

int get_index() {
  if (clients >= MAX_CLIENTS) {
    return -1;
  }

  int index = get_available_thread_index();
  return index;
}

void broadcast(char* message) {
  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i] != NULL) {
      send(connections[i]->client_fd, message, BUFF_SIZE, 0);
    }
  }
}

void* create_worker_args(int index, socket_t client_fd) {
  worker_args_t* args = (worker_args_t*)malloc(sizeof(worker_args_t));
  args->index = index;
  args->client_fd = client_fd;
  return (void*)args;
}

bool is_valid_equipment(equipment_t equipment) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i] != NULL && connections[i]->client_fd == equipment) {
      return TRUE;
    }
  }
  return FALSE;
}

// ======================= Actions ======================= //

void disconnect_client(socket_t client_fd) {
  char* dc_message = (char*)calloc(BUFF_SIZE, sizeof(char));
  send(client_fd, dc_message, BUFF_SIZE, 0);
  close(client_fd);
}

void send_error(socket_t client_fd, char* error_code) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();

  res_args->id = ERROR;
  res_args->destination = client_fd;
  set_payload(res_args, error_code, sizeof(error_code));

  encode_args(res, res_args);
  send(client_fd, res, BUFF_SIZE, 0);
  destroy_message(res_args);
}

void send_max_clients_reached(socket_t client_fd){
  send_error(client_fd, MAX_CLIENTS_REACHED_PAYLOAD);
}

void send_remove_error(socket_t client_fd) {
  send_error(client_fd, REMOVE_ERROR_PAYLOAD);
}

void send_origin_equipment_not_found(socket_t client_fd) {
  send_error(client_fd, ORIGIN_EQUIPMENT_NOT_FOUND_PAYLOAD);
}

void send_destination_equipment_not_found(socket_t client_fd) {
  send_error(client_fd, DESTINATION_EQUIPMENT_NOT_FOUND_PAYLOAD);
}

void list_equipments(worker_args_t *args) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();

  char payload[(MAX_CLIENTS * 2) + 1];
  res_args->id = RES_LIST;
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i] != NULL && connections[i]->client_fd != args->client_fd) {
      sprintf(payload, "%s%02d", payload, connections[i]->client_fd);
    }
  }

  set_payload(res_args, payload, strlen(payload));
  encode_args(res, res_args);
  send(args->client_fd, res, BUFF_SIZE, 0);
}

void broadcast_remove(int client){
  char req[BUFF_SIZE];
  message_t* req_args = init_message();

  req_args->id = REQ_REM;
  req_args->origin = client;

  encode_args(req, req_args);
  destroy_message(req_args);
  broadcast(req);
}

int request_quipment_info(message_t* message, message_t* res_args) {
  char req[BUFF_SIZE], res[BUFF_SIZE];

  encode_args(req, message);
  send(message->destination, req, BUFF_SIZE, 0);

  if(recv(message->destination, res, BUFF_SIZE, 0) == END_OF_COM){
    return -1;
  }

  decode_args(res, res_args);
  if(res_args->id != RES_INF) {
    return 1;
  }

  return 0;
}

// ======================= Handlers ======================= //

void handle_remove(message_t* message) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();
  int origin = message->origin, index = -1;

  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (connections[i] != NULL && connections[i]->client_fd == origin) {
      index = i;
      break;
    }
  }

  if(index == -1){
    send_remove_error(message->origin);
    return;
  }

  pthread_mutex_lock(&mutex);
  worker_args_t* client = connections[index];
  connections[index] = NULL;
  threads[index] = 0;
  clients--;
  broadcast_remove(origin);
  pthread_mutex_unlock(&mutex);

  res_args->id = OK;
  res_args->destination = origin;
  set_payload(res_args, SUCCESSFULL_REMOVAL_PAYLOAD, sizeof(SUCCESSFULL_REMOVAL_PAYLOAD));
  encode_args(res, res_args);

  send(client->client_fd, res, BUFF_SIZE, 0);
  destroy_message(res_args);
  close(origin);
  free(client);
}

void handle_req_inf(message_t* message, socket_t client_fd) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();
  int origin = message->origin, destination = message->destination;

  pthread_mutex_lock(&mutex);
  if(!is_valid_equipment(origin)) {
    printf("Equipment %02d not found\n", origin);
    send_origin_equipment_not_found(client_fd);
    pthread_mutex_unlock(&mutex);
    return;
  }

  if(!is_valid_equipment(destination)) {
    printf("Equipment %02d not found\n", destination);
    send_destination_equipment_not_found(client_fd);
    pthread_mutex_unlock(&mutex);
    return;
  }
  pthread_mutex_unlock(&mutex);

  if (request_quipment_info(message, res_args) != 0) {
    // send_error(client_fd, REQUEST_ERROR_PAYLOAD);
    return;
  }

  pthread_mutex_lock(&mutex);
  if (!is_valid_equipment(res_args->origin)) {
    printf("Equipment %02d not found\n", res_args->origin);
    send_origin_equipment_not_found(res_args->origin);
    pthread_mutex_unlock(&mutex);
    return;
  }

  if (!is_valid_equipment(res_args->destination)) {
    printf("Equipment %02d not found\n", res_args->destination);
    send_destination_equipment_not_found(res_args->destination);
    pthread_mutex_unlock(&mutex);
    return;
  }
  pthread_mutex_unlock(&mutex);

  encode_args(res, res_args);
  send(client_fd, res, BUFF_SIZE, 0);
  destroy_message(res_args);
}

void handle_request(message_t* request, socket_t client_fd) {
  switch (request->id) {
    case REQ_INF:
      handle_req_inf(request, client_fd);
      break;

    case REQ_REM:
      handle_remove(request);
      break;
    
    default:
      // 
      break;
  }
}

// ======================= Threads ======================= //

void* worker(void* arg) {
  char req[BUFF_SIZE];
  char res[BUFF_SIZE];
  message_t* req_args = init_message();
  message_t* res_args = init_message();
  worker_args_t* client = (worker_args_t*)arg;
  int client_fd = client->client_fd;
  int index = client->index;
  printf("Equipment %02d added\n", client_fd);

  list_equipments(client);
  while (connections[index] != NULL && read(client_fd, req, BUFF_SIZE) != END_OF_COM) {
    decode_args(req, req_args);
    handle_request(req_args, client_fd);
  }

  destroy_message(req_args);
  destroy_message(res_args);
}

void create_client(socket_t client_fd, int index) {
  char res[BUFF_SIZE];
  message_t* res_args = init_message();
  worker_args_t* worker_args = create_worker_args(index, client_fd);

  clients++;
  connections[index] = worker_args;

  res_args->id = RES_ADD;
  char payload[3];
  sprintf(payload, "%02d", client_fd);
  set_payload(res_args, payload, sizeof(payload));
  encode_args(res, res_args);
  broadcast(res);

  pthread_create(&threads[index], NULL, worker, worker_args);

  destroy_message(res_args);
}

void handshake(socket_t client_fd) {
  char req[BUFF_SIZE];
  int index;
  message_t* req_args = init_message();

  if (read(client_fd, req, BUFF_SIZE) != END_OF_COM) {
    decode_args(req, req_args);
    if (req_args->id != REQ_ADD) {
      disconnect_client(client_fd);
    }
    else {
      pthread_mutex_lock(&mutex);
      index = get_index();
      if (index == -1) {
        printf("Too many clients\n");
        send_max_clients_reached(client_fd);
        close(client_fd);
      }
      else {
        create_client(client_fd, index);
      }
      pthread_mutex_unlock(&mutex);
    }
  }

  destroy_message(req_args);
}

// ======================= Main ======================= //

int main(int argc, char const *argv[])
{
  int port = atoi(argv[1]);
  pthread_mutex_init(&mutex, NULL);

  server_socket_info_t* sock_info = create_server_socket(port);
  printf("Server listening on port %d\n", port);
  
  while(TRUE){
    socket_t client_fd = connect_client(sock_info);
    handshake(client_fd);
  }

  pthread_mutex_destroy(&mutex);
  close(sock_info->fd);

  return 0;
}
