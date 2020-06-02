#include "comms.h"
#include "UI_library.h"
#include "pacman.h"

#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8000
#define DEBUG 1

int client_sockets[100];
int n_clients;

typedef struct client_args {
  int client_socket;
} client_args;

typedef struct event_struct {
  int dir;   // 0:down | 1:up | 2:left | 3:right |
  int type;  // 0:keyboard | 1:mouse |
  int client_id;
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

Uint32 Event_Move, Event_NewUser, Event_Disconnect;

void send_NewClient(int n_clients, entity** pacmans, entity** monsters) {
  int message_type = 0, socket;
  coords pacman_coords, monster_coords;
  rgb client_color;

  pacman_coords.x = pacmans[n_clients - 1]->line;
  pacman_coords.y = pacmans[n_clients - 1]->column;
  monster_coords.x = monsters[n_clients - 1]->line;
  monster_coords.y = monsters[n_clients - 1]->column;
  client_color.r = pacmans[n_clients - 1]->u_details->r;
  client_color.g = pacmans[n_clients - 1]->u_details->g;
  client_color.b = pacmans[n_clients - 1]->u_details->b;

  if (DEBUG)
    printf(
        "Sending new client\npacman coords: %d %d monster coords: %d %d rgb: "
        "%d %d %d\n",
        pacman_coords.x, pacman_coords.y, monster_coords.x, monster_coords.y,
        client_color.r, client_color.g, client_color.b);

  for (int i = 0; i < n_clients; i++) {
    socket = pacmans[i]->u_details->client_socket;
    send(socket, &message_type, sizeof(int), 0);
    send(socket, &client_color, sizeof(rgb), 0);
    send(socket, &pacman_coords, sizeof(pacman_coords), 0);
    send(socket, &monster_coords, sizeof(monster_coords), 0);
  }
}

void rcv_NewClient(client_data** clients, int server_socket, int* n_users) {
  client_data* client = malloc(sizeof(client_data));
  rgb* client_color = malloc(sizeof(rgb));
  coords* pacman_coords = malloc(sizeof(coords));
  coords* monster_coords = malloc(sizeof(coords));

  recv(server_socket, client_color, sizeof(rgb), 0);
  recv(server_socket, pacman_coords, sizeof(coords), 0);
  recv(server_socket, monster_coords, sizeof(coords), 0);

  client->rgb = client_color;
  client->pacman_coords = pacman_coords;
  client->monster_coords = monster_coords;

  clients[*n_users] = client;
  if (DEBUG)
    printf(
        "Received new client\npacman coords: %d %d monster coords: %d %d rgb: "
        "%d %d %d\n",
        client->pacman_coords->x, client->pacman_coords->y,
        client->monster_coords->x, client->monster_coords->y, client->rgb->r,
        client->rgb->g, client->rgb->b);
  paint_pacman(client->pacman_coords->y, client->pacman_coords->x,
               client->rgb->r, client->rgb->g, client->rgb->b);
  paint_monster(client->monster_coords->y, client->monster_coords->x,
                client->rgb->r, client->rgb->g, client->rgb->b);
  (*n_users)++;
}

void send_Disconnect(int client_id, int n_clients, entity** pacmans) {
  int message_type = 1, socket;
  for (int i = 0; i < n_clients; i++) {
    socket = pacmans[i]->u_details->client_socket;
    send(socket, &message_type, sizeof(int), 0);
    send(socket, &client_id, sizeof(client_id), 0);
  }
}

void rcv_Disconnect(client_data** clients, int server_socket, int* n_clients) {
  int client_id;

  printf("Received disconnect: %d\n", client_id);
  recv(server_socket, &client_id, sizeof(int), 0);
  clear_place(clients[client_id]->pacman_coords->y,
              clients[client_id]->pacman_coords->x);
  clear_place(clients[client_id]->monster_coords->y,
              clients[client_id]->monster_coords->x);

  free(clients[client_id]->rgb);
  free(clients[client_id]->pacman_coords);
  free(clients[client_id]->monster_coords);
  free(clients[client_id]);

  clients[client_id] = clients[*n_clients - 1];
  (*n_clients)--;
}

void send_AllClients(int n_clients, entity** pacmans, entity** monsters) {
  int message_type = 2, socket;
  coords pacman_coords, monster_coords;
  rgb client_color;

  if (DEBUG)
    printf("Sending all clients: %d\n", n_clients);

  socket = pacmans[n_clients]->u_details->client_socket;
  send(socket, &message_type, sizeof(int), 0);
  send(socket, &n_clients, sizeof(int), 0);

  for (int i = 0; i < n_clients; i++) {
    pacman_coords.x = pacmans[i]->line;
    pacman_coords.y = pacmans[i]->column;
    monster_coords.x = monsters[i]->line;
    monster_coords.y = monsters[i]->column;
    client_color.r = pacmans[i]->u_details->r;
    client_color.g = pacmans[i]->u_details->g;
    client_color.b = pacmans[i]->u_details->b;

    if (DEBUG)
      printf("pacman coords: %d %d monster coords: %d %d rgb: %d %d %d\n",
             pacman_coords.x, pacman_coords.y, monster_coords.x,
             monster_coords.y, pacmans[i]->u_details->r,
             pacmans[i]->u_details->g, pacmans[i]->u_details->b);

    send(socket, &client_color, sizeof(rgb), 0);
    send(socket, &pacman_coords, sizeof(coords), 0);
    send(socket, &monster_coords, sizeof(coords), 0);
  }
}

void rcv_AllClients(client_data** clients, int server_socket, int* n_clients) {
  client_data* client;
  rgb* client_color;
  coords* pacman_coords;
  coords* monster_coords;

  recv(server_socket, n_clients, sizeof(int), 0);
  printf("Recieving all clients: %d\n", *n_clients);

  for (int i = 0; i < *n_clients; i++) {
    client_color = malloc(sizeof(rgb));
    recv(server_socket, client_color, sizeof(rgb), 0);

    pacman_coords = malloc(sizeof(coords));
    recv(server_socket, pacman_coords, sizeof(coords), 0);

    monster_coords = malloc(sizeof(coords));
    recv(server_socket, monster_coords, sizeof(coords), 0);

    client = malloc(sizeof(client_data));
    client->rgb = client_color;
    client->pacman_coords = pacman_coords;
    client->monster_coords = monster_coords;

    clients[i] = client;
    paint_pacman(client->pacman_coords->y, client->pacman_coords->x,
                 client->rgb->r, client->rgb->g, client->rgb->b);
    paint_monster(client->monster_coords->y, client->monster_coords->x,
                  client->rgb->r, client->rgb->g, client->rgb->b);
    if (DEBUG)
      printf("pacman coords: %d %d monster coords: %d %d rgb: %d %d %d\n",
             client->pacman_coords->x, client->pacman_coords->y,
             client->monster_coords->x, client->monster_coords->y,
             client->rgb->r, client->rgb->g, client->rgb->b);
  }
}

void send_AllBricks(int client_socket, entity** bricks, int n_bricks) {
  int message_type = 3;
  coords brick;

  if (DEBUG)
    printf("Sending all bricks\n");
  send(client_socket, &message_type, sizeof(int), 0);
  send(client_socket, &n_bricks, sizeof(int), 0);

  for (int i = 0; i < n_bricks; i++) {
    brick.x = bricks[i]->line;
    brick.y = bricks[i]->column;

    if (DEBUG)
      printf("brick coords: %d %d\n", brick.x, brick.y);
    send(client_socket, &brick, sizeof(coords), 0);
  }
}

void rcv_AllBricks(int server_socket) {
  int n_bricks;
  coords* brick = malloc(sizeof(coords));

  recv(server_socket, &n_bricks, sizeof(int), 0);

  for (int i = 0; i < n_bricks; i++) {
    recv(server_socket, brick, sizeof(coords), 0);
    if (DEBUG)
      printf("brick coords: %d %d\n", brick->x, brick->y);
    paint_brick(brick->y, brick->x);
  }
  free(brick);
}

void send_AllFruits(int client_socket, entity** fruits, int n_fruits) {
  int message_type = 4;
  coords fruit;

  if (DEBUG)
    printf("Sending all fruits\n");
  send(client_socket, &message_type, sizeof(int), 0);
  send(client_socket, &n_fruits, sizeof(int), 0);

  for (int i = 0; i < n_fruits; i++) {
    fruit.x = fruits[i]->line;
    fruit.y = fruits[i]->column;

    if (DEBUG)
      printf("fruits coords: %d %d\n", fruit.x, fruit.y);
    send(client_socket, &fruit, sizeof(coords), 0);
    send(client_socket, &fruits[i]->type, sizeof(int), 0);
  }
}

void rcv_AllFruits(int server_socket) {
  int n_fruits, type;
  coords* fruit = malloc(sizeof(coords));

  recv(server_socket, &n_fruits, sizeof(int), 0);

  for (int i = 0; i < n_fruits; i++) {
    recv(server_socket, fruit, sizeof(coords), 0);
    recv(server_socket, &type, sizeof(int), 0);
    if (DEBUG)
      printf("fruit coords: %d %d\n", fruit->x, fruit->y);
    if (type) {
      paint_lemon(fruit->y, fruit->x);
    } else {
      paint_cherry(fruit->y, fruit->x);
    }
  }
  free(fruit);
}

void send_Move(int* updated, entity** pacmans, int n_clients) {
  int message_type = 5, i, j;

  for (i = 0; i < n_clients; i++) {
    send(pacmans[i]->u_details->client_socket, &message_type, sizeof(int), 0);
  }
}

void* serverThread(void* args) {
  int* server_socket = args;
  int err_rcv, message_type;
  client_data* clients[100];
  int n_clients = 0;

  coords board_size;
  recv(*server_socket, &board_size, sizeof(coords), 0);
  printf("board: %d %d\n", board_size.x, board_size.y);
  create_board_window(board_size.y, board_size.x);

  printf("here\n");
  while ((err_rcv = recv(*server_socket, &message_type, sizeof(int), 0)) > 0) {
    printf("message type: %d\n", message_type);
    if (message_type == 0) {
      rcv_NewClient(clients, *server_socket, &n_clients);
    } else if (message_type == 1) {
      rcv_Disconnect(clients, *server_socket, &n_clients);
    } else if (message_type == 2) {
      rcv_AllClients(clients, *server_socket, &n_clients);
    } else if (message_type == 3) {
      rcv_AllBricks(*server_socket);
    } else if (message_type == 4) {
      rcv_AllFruits(*server_socket);
    }
  }

  return (NULL);
}

void* clientThread(void* args) {
  event_message msg;
  int err_rcv;
  event_struct* event_data;
  SDL_Event new_event;
  entity* client = args;

  if (DEBUG)
    printf("just connected to the server\n");

  while ((err_rcv = recv(client->u_details->client_socket, &msg,
                         sizeof(event_message), 0)) > 0) {
    if (DEBUG)
      printf("received %d type: %d dir: %d\n", err_rcv, msg.type, msg.dir);
    if (msg.dir >= 0 && msg.dir <= 4) {
      event_data = malloc(sizeof(event_struct));
      event_data->dir = msg.dir;
      event_data->client_id = client->idx;
      event_data->type = msg.type;

      SDL_zero(new_event);
      new_event.type = Event_Move;
      new_event.user.data1 = event_data;
      SDL_PushEvent(&new_event);
    }
  }

  SDL_zero(new_event);
  new_event.type = Event_Disconnect;
  new_event.user.data1 = client;
  SDL_PushEvent(&new_event);
  return (NULL);
}

void* connectionThread() {
  int server_socket, client_socket, err, r, g, b;
  struct sockaddr_in server_local_addr, client_addr;
  socklen_t size_addr = sizeof(client_addr);
  user_details* client;
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
    recv(client_socket, &r, sizeof(int), 0);
    recv(client_socket, &g, sizeof(int), 0);
    recv(client_socket, &b, sizeof(int), 0);
    client = get_newUser(client_socket, r, g, b);
    new_event.user.data1 = client;
    SDL_PushEvent(&new_event);
  }
}

void handle_NewUser(user_details* new_client_details,
                    entity** pacmans,
                    entity** monsters,
                    entity** free_spaces,
                    entity** bricks,
                    entity** fruits,
                    int* n_clients,
                    int* n_free_spaces,
                    int n_bricks,
                    int n_fruits,
                    int n_lines,
                    int n_cols) {
  entity* new_entity;
  int random_idx;
  pthread_t thread_id;
  coords board_size;

  // get random free space for entering pacman
  random_idx = random() % *n_free_spaces;
  new_entity = free_spaces[random_idx];
  new_entity->type = 2;
  new_entity->idx = *n_clients;
  new_entity->u_details = new_client_details;
  pacmans[*n_clients] = new_entity;
  free_spaces[random_idx] = free_spaces[*n_free_spaces - 1];
  free_spaces[random_idx]->idx = random_idx;
  (*n_free_spaces)--;

  pthread_create(&thread_id, NULL, clientThread, pacmans[*n_clients]);
  // get random free space for entering monster
  random_idx = random() % *n_free_spaces;
  new_entity = free_spaces[random_idx];
  new_entity->type = 3;
  new_entity->idx = *n_clients;
  new_entity->u_details = new_client_details;
  monsters[*n_clients] = new_entity;
  free_spaces[random_idx] = free_spaces[*n_free_spaces - 1];
  free_spaces[random_idx]->idx = random_idx;
  (*n_free_spaces)--;

  board_size.x = n_lines;
  board_size.y = n_cols;
  send(pacmans[*n_clients]->u_details->client_socket, &board_size,
       sizeof(coords), 0);
  if (*n_clients > 0)
    send_AllClients(*n_clients, pacmans, monsters);
  if (n_bricks > 0)
    send_AllBricks(pacmans[*n_clients]->u_details->client_socket, bricks,
                   n_bricks);
  if (n_fruits > 0)
    send_AllFruits(pacmans[*n_clients]->u_details->client_socket, fruits,
                   n_fruits);

  (*n_clients)++;

  send_NewClient(*n_clients, pacmans, monsters);
}

void handle_Disconnect(entity** pacmans,
                       entity** monsters,
                       entity** free_spaces,
                       int n_lines,
                       int n_cols,
                       int client_id,
                       int* n_free_spaces,
                       int* n_clients) {
  entity* new_entity;

  if (DEBUG)
    printf("disconnecting client %d\n", client_id);
  // get new free space with coords of leaving pacman
  new_entity = pacmans[client_id];
  new_entity->type = -1;
  new_entity->idx = *n_free_spaces;
  close(new_entity->u_details->client_socket);
  free(new_entity->u_details);
  free_spaces[*n_free_spaces] = new_entity;
  (*n_free_spaces)++;

  // get new free space with coords of leaving monster
  new_entity = monsters[client_id];
  new_entity->type = -1;
  new_entity->idx = *n_free_spaces;
  free_spaces[*n_free_spaces] = new_entity;
  (*n_free_spaces)++;

  // move last member of array to leaving idx
  pacmans[client_id] = pacmans[*n_clients - 1];
  pacmans[client_id]->idx = client_id;
  monsters[client_id] = monsters[*n_clients - 1];
  monsters[client_id]->idx = client_id;

  (*n_clients)--;

  send_Disconnect(client_id, *n_clients, pacmans);
}

int main(int argc, char* argv[]) {
  int done = 0, n_lines = 100, n_cols = 100;
  SDL_Event event;
  pthread_t thread_id;
  entity *pacmans[100], *monsters[100], **bricks, **free_spaces, *fruits[100];
  entity*** board;
  int n_clients = 0, n_bricks = 0, n_free_spaces = 0, n_fruits = 0;
  char c;

  Event_Move = SDL_RegisterEvents(1);
  Event_NewUser = SDL_RegisterEvents(1);
  Event_Disconnect = SDL_RegisterEvents(1);

  if (argc == 2) {  // server
    FILE* fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
      printf("error: cannot open file\n");
      exit(1);
    }

    fscanf(fp, "%d %d", &n_cols, &n_lines);
    getc(fp);

    board = (entity***)malloc(sizeof(entity**) * n_lines);

    for (int i = 0; i < n_lines; i++) {
      board[i] = (entity**)malloc(sizeof(entity*) * n_cols);
    }

    for (int i = 0; i < n_lines; i++) {
      for (int a = 0; a < n_cols; a++) {
        c = getc(fp);
        if (c == 'B') {
          board[i][a] = get_newEntity(i, a, 6, n_bricks, NULL);
          n_bricks++;
        }
        if (c == ' ') {
          board[i][a] = get_newEntity(i, a, -1, n_free_spaces, NULL);
          n_free_spaces++;
        }
        if (c == 'L') {
          board[i][a] = get_newEntity(i, a, 0, n_fruits, NULL);
          n_fruits++;
        }
        if (c == 'C') {
          board[i][a] = get_newEntity(i, a, 1, n_fruits, NULL);
          n_fruits++;
        }
      }
      getc(fp);
    }

    fclose(fp);

    bricks = (entity**)malloc(sizeof(entity*) * n_bricks);
    free_spaces = (entity**)malloc(sizeof(entity*) * n_free_spaces);

    int aux1 = 0;
    int aux2 = 0;
    int aux3 = 0;

    for (int i = 0; i < n_lines; i++) {
      for (int a = 0; a < n_cols; a++) {
        if ((board[i][a])->type == 6) {  //é brick

          bricks[aux1] = board[i][a];
          aux1++;
        } else if (board[i][a]->type == -1) {  //é free space
          free_spaces[aux2] = board[i][a];
          aux2++;
        } else if (board[i][a]->type == 0 || board[i][a]->type == 1) {
          fruits[aux3] = board[i][a];
          aux3++;
        }
      }
    }
    create_board_window(n_cols, n_lines);

    for (int i = 0; i < n_lines; i++) {
      for (int a = 0; a < n_cols; a++) {
        if ((board[i][a]) != NULL) {
          if ((board[i][a])->type == 6)  // brick
          {
            paint_brick(a, i);
          }
          if ((board[i][a])->type == 0)  // cherry
          {
            // paint cherry
          }
          if ((board[i][a])->type == 1)  // lemon
          {
            // paint lemon
          }
          if ((board[i][a])->type == 2)  // Pacman
          {
            // paint cherry
          }
          if ((board[i][a])->type == 2)  // Monster
          {
            // paint Monster
          }
          if ((board[i][a])->type == 4)  // Charged_Pacman
          {
            // paint Charged_Pacman
          }

        }  // 0:Cherry | 1:Lemon | 2:Pacman | 3:Monster | (MAXINT-1:Wall) |
           // MAXINT:Wall |
      }
    }

    pthread_create(&thread_id, NULL, connectionThread, NULL);

    while (!done) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = SDL_TRUE;
        } else if (event.type == Event_Move) {
          //  TODO: handle move event
        } else if (event.type == Event_NewUser) {
          user_details* client;
          client = event.user.data1;
          if (DEBUG)
            printf("fd %d\n", client->client_socket);
          handle_NewUser(client, pacmans, monsters, free_spaces, bricks, fruits,
                         &n_clients, &n_free_spaces, n_bricks, n_fruits,
                         n_lines, n_cols);
        } else if (event.type == Event_Disconnect) {
          entity* client;
          client = event.user.data1;
          handle_Disconnect(pacmans, monsters, free_spaces, n_lines, n_cols,
                            client->idx, &n_free_spaces, &n_clients);
        }
      }
    }
  } else if (argc == 6) {  // client
    struct sockaddr_in server_addr;
    int sock_fd, r, g, b;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
      perror("socket: ");
      exit(-1);
    }

    server_addr.sin_family = AF_INET;
    int port_number;
    if (sscanf(argv[2], "%d", &port_number) != 1) {
      printf("argv[2] is not a number\n");
      exit(-1);
    }
    server_addr.sin_port = htons(port_number);
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
      printf("argv[1]is not a valida address\n");
      exit(-1);
    }
    if (sscanf(argv[3], "%d", &r) != 1) {
      printf("argv[2] is not a number\n");
      exit(-1);
    }
    if (sscanf(argv[4], "%d", &g) != 1) {
      printf("argv[2] is not a number\n");
      exit(-1);
    }
    if (sscanf(argv[5], "%d", &b) != 1) {
      printf("argv[2] is not a number\n");
      exit(-1);
    }

    printf("connecting to %s %d\n", argv[1], server_addr.sin_port);

    if (-1 == connect(sock_fd, (const struct sockaddr*)&server_addr,
                      sizeof(server_addr))) {
      printf("Error connecting\n");
      exit(-1);
    }

    send(sock_fd, &r, sizeof(int), 0);
    send(sock_fd, &g, sizeof(int), 0);
    send(sock_fd, &b, sizeof(int), 0);

    pthread_create(&thread_id, NULL, serverThread, &sock_fd);
    usleep(1000000);

    event_message move_msg;
    int x = 4, y = 4;

    while (!done) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          done = SDL_TRUE;
        } else if (event.type == SDL_KEYDOWN) {
          move_msg.type = 0;
          if (event.key.keysym.sym == SDLK_DOWN) {
            printf("down!\n");
            move_msg.dir = 0;
          } else if (event.key.keysym.sym == SDLK_UP) {
            printf("up!\n");
            move_msg.dir = 1;
          } else if (event.key.keysym.sym == SDLK_LEFT) {
            printf("left!\n");
            move_msg.dir = 2;
          } else if (event.key.keysym.sym == SDLK_RIGHT) {
            printf("right!\n");
            move_msg.dir = 3;
          }
          send(sock_fd, &move_msg, sizeof(event_message), 0);
        } else if (event.type == SDL_MOUSEMOTION) {
          move_msg.type = 1;
          int x_new, y_new;
          // this fucntion return the place cwher the mouse cursor is
          get_board_place(event.motion.x, event.motion.y, &x_new, &y_new);
          // if the mluse moved toi anothe place
          if ((x_new != x) || (y_new != y)) {
            x = x_new;
            y = y_new;
            if (x_new - x > 0) {
              printf("right! %d\n", x_new);
              move_msg.dir = 3;
            } else if (x_new - x < 0) {
              printf("left!\n");
              move_msg.dir = 2;
            } else if (y_new - y > 0) {
              printf("down!\n");
              move_msg.dir = 0;
            } else if (y_new - y < 0) {
              printf("up!\n");
              move_msg.dir = 1;
            }
            send(sock_fd, &move_msg, sizeof(event_message), 0);
          }
        }
      }
    }
  } else {
    exit(-1);
  }
}
