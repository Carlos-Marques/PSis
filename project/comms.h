#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "UI_library.h"
#include "pacman.h"

#define PORT 8000
#define DEBUG 1

typedef struct server_args {
  int server_socket;
  int ready;
  int x_pacman;
  int y_pacman;
} server_args;

typedef struct client_args {
  int client_socket;
} client_args;

typedef struct event_struct {
  int dir;   // 0:down | 1:up | 2:left | 3:right |
  int type;  // 0:keyboard | 1:mouse |
  entity* client;
} event_struct;

typedef struct event_message {
  int type;  // 0:keyboard | 1:mouse |
  int dir;
} event_message;

typedef struct coords {
  int x;
  int y;
} coords;

typedef struct rgb {
  int r;
  int g;
  int b;
} rgb;

typedef struct client_data {
  rgb* rgb;
  coords* pacman_coords;
  coords* monster_coords;
} client_data;

typedef struct characters {
  entity* pacman;
  entity* monster;
} characters;
