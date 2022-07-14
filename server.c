#include <pthread.h>
#include <stdio.h>

#include"./common.h"

#define MAX_CLIENTS 15

pthread_t threads[MAX_CLIENTS];
equipement_t equipements[MAX_CLIENTS];
pthread_mutex_t mutex;

typedef struct worker_args {
  int index;
  socket_t client_fd;
} worker_args_t;

int get_available_thread_index() {
  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (threads[i] == 0) {
      return i;
    }
  }
  return -1;
}

void* create_worker_args(int index, socket_t client_fd) {
  worker_args_t* args = (worker_args_t*) malloc(sizeof(worker_args_t));
  args->index = index;
  args->client_fd = client_fd;
  return (void*)args;
}

void disconnect_client(worker_args_t* worker_args) {
  char* dc_message = (char*)calloc(BUFF_SIZE, sizeof(char));
  send(worker_args->client_fd, dc_message, BUFF_SIZE, 0);
  close(worker_args->client_fd);
}

int handshake(worker_args_t* worker_args) {
  char req[BUFF_SIZE];
  char res[BUFF_SIZE];
  int code = 0;
  message_t* req_args = init_message();
  message_t* res_args = init_message();

  if (read(worker_args->client_fd, req, BUFF_SIZE) != 0b00000000) {
    decode_args(req, req_args);
    if (req_args->id != 1) {
      printf("Iconrrect handshake payload\n");
      code = -1;
    }
    else{
      res_args->id = 3;
      char payload[3];
      sprintf(payload, "%02d", worker_args->client_fd);
      set_payload(res_args, payload, sizeof(payload));
      encode_args(res, res_args);
      send(worker_args->client_fd, res, BUFF_SIZE, 0);
    }
  }
  else{
    code = -1;
  }
  destroy_message(req_args);
  destroy_message(res_args);
  return code;
}

void* worker(void* arg) {
  char buff[BUFF_SIZE];
  worker_args_t* args = (worker_args_t*)arg;
  printf("Connected client %d\n", args->client_fd);

  if(handshake(args) != 0){
    printf("Handshake %d failed\n", args->client_fd);
    disconnect_client(args);
    return NULL;
  }
  
  while (read(args->client_fd, buff, BUFF_SIZE) != 0b00000000) {
    // printf("< %s\n", buff);
  }
  return NULL;
}

int main(int argc, char const *argv[])
{
  int port = atoi(argv[1]), clients = 0;
  pthread_mutex_init(&mutex, NULL);

  server_socket_info_t* sock_info = create_server_socket(port);
  printf("Server listening on port %d\n", port);
  
  while(TRUE){
    socket_t client_fd = connect_client(sock_info);

    pthread_mutex_lock(&mutex);
    clients++;
    int index = get_available_thread_index();
    if (index == -1) {
      printf("Too many clients\n");
      exit(6);
    }
    pthread_create(&threads[index], NULL, worker, create_worker_args(index, client_fd));
    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_destroy(&mutex);
  close(sock_info->fd);

  return 0;
}
