#include <pthread.h>
#include <stdio.h>

#include"./common.h"

#define MAX_CLIENTS 15

pthread_t threads[MAX_CLIENTS];
equipement_t equipements[MAX_CLIENTS];

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

void* worker(void* arg) {
  char buff[BUFF_SIZE];
  worker_args_t* args = (worker_args_t*)arg;
  printf("Connected client %d\n", args->client_fd);
  while (recv(args->client_fd, buff, BUFF_SIZE, 0) != 0b00000000) {
    // 
  }
  return NULL;
}

int main(int argc, char const *argv[])
{
  int port = atoi(argv[1]), clients = 0;
  printf("Server listening on port %d\n", port);
  
  while(TRUE){
    server_socket_info_t* sock_info = create_server_socket(port);
    socket_t client_fd;

    if ((client_fd = accept(sock_info->fd, sock_info->addr, &(sock_info->addr_len))) < 0) {
      perror("accept");
      exit(5);
    }

    clients++;
    int index = get_available_thread_index();
    if (index == -1) {
      printf("Too many clients\n");
      exit(6);
    }

    pthread_create(&threads[index], NULL, worker, create_worker_args(index, client_fd));
  }

  return 0;
}
