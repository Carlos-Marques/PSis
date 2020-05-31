#include "comms.h"

#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#define PORT 1337
#define DEBUG 1

int client_sockets[100];
int n_clients;

typedef struct client_args {
  int client_socket;
  int client_id;
} client_args;

typedef struct event_struct {
  int x;
  int y;
  int client_id;
} event_struct;

typedef struct event_message {
  int type;  // 0:keyboard | 1:mouse |
  int x;
  int y;
} event_message;

Uint32 Event_MovePacman, Event_MoveMonster, Event_NewUser, Event_Disconnect;

void* clientThread(void* args) {
  event_message msg;
  int err_rcv;
  event_struct* event_data;
  SDL_Event new_event;
  client_args* client = args;

  if (DEBUG)
    printf("just connected to the server\n");
  while ((err_rcv = recv(client->client_socket, &msg, sizeof(msg), 0)) > 0) {
    if (DEBUG)
      printf("received %d byte %d %d %d\n", err_rcv, msg.type, msg.x, msg.y);
    event_data = malloc(sizeof(event_struct));
    event_data->x = msg.x;
    event_data->y = msg.y;
    event_data->client_id = client->client_id;

    SDL_zero(new_event);
    if (msg.type) {
      new_event.type = Event_MovePacman;
    } else {
      new_event.type = Event_MoveMonster;
    }
    new_event.user.data1 = event_data;
    SDL_PushEvent(&new_event);
  }

  SDL_zero(new_event);
  new_event.type = Event_Disconnect;
  new_event.user.data1 = client;
  SDL_PushEvent(&new_event);
  return (NULL);
}

void* connectionThread() {
  int server_socket, client_socket, err;
  struct sockaddr_in server_local_addr, client_addr;
  socklen_t size_addr = sizeof(client_addr);
  client_args* client = malloc(sizeof(client_args));
  pthread_t thread_id;
  SDL_Event new_event;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);  // TCP socket
  if (server_socket == -1) {
    perror("SOCKET");
    exit(EXIT_FAILURE);
  }

  server_local_addr.sin_family = AF_INET;
  server_local_addr.sin_addr.s_addr = INADDR_ANY;
  server_local_addr.sin_port = htons(PORT);

  err = bind(server_socket, (struct sockaddr*)&server_local_addr,
             sizeof(server_local_addr));
  if (err == -1) {
    perror("bind");
    exit(-1);
  }

  if (listen(server_socket, 5) == -1) {
    perror("listen");
    exit(-1);
  }

  while (1) {
    if (DEBUG)
      printf("waiting for connections\n");
    client_socket =
        accept(server_socket, (struct sockaddr*)&client_addr, &size_addr);
    if (client_socket == -1) {
      perror("ACEPT");
      exit(EXIT_FAILURE);
    }
    if (DEBUG)
      printf("accepted connection \n");

    SDL_zero(new_event);
    new_event.type = Event_NewUser;
    client->client_socket = client_socket;
    new_event.user.data1 = client;
    // TODO: send board size and bricks
  }
}

void handle_NewUser(client_args* client) {}

int main(int argc, char* argv[]) {
  int done = 0;
  SDL_Event event;
  pthread_t thread_id;

  Event_MovePacman = SDL_RegisterEvents(1);
  Event_MoveMonster = SDL_RegisterEvents(1);
  Event_NewUser = SDL_RegisterEvents(1);
  Event_Disconnect = SDL_RegisterEvents(1);

  if (argc == 2) {  // server
    pthread_create(&thread_id, NULL, connectionThread, NULL);
  } else if (argc == 3) {  // client
  } else {
    exit(-1);
  }

  // TODO: create board and print first time

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      } else if (event.type == Event_MovePacman) {
        //  TODO: handle pacman event
      } else if (event.type == Event_MoveMonster) {
        // TODO: handle monster event
      } else if (event.type == Event_NewUser) {
        // TODO: handle new user event
      } else if (event.type == Event_Disconnect) {
        // TODO: handle disconnect
      }
    }
  }
}
