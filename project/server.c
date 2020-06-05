#include "server.h"

// DONE: add fruits - Carlos
// DONE: send new fruit - Carlos
// DONE: limit colors - Espadinha
// DONE: limit number of players to free spaces - Carlos
// DONE: check sends and recvs - Espadinha
// DONE: fix mouse movement when too quick - Carlos
// DON: fix movement delay - Carlos
// DONE: fix server.x updates - Carlos
// DONE: fix pacman and monster switch in client - Carlos
// DONE: send score - Espadinha
// DONE: superpacman message - Carlos
// DONE: fix handle move - Espadinha
// DONE: frees - Espadinha
// DONE: kills client threads
// DONE: kill scoreboard thread
// DONE: kills inactivity threads
// DONE: kill connection thread
// DONE: kill server thread in client
// DONE: handle server disconnect
// DONE: handle send errors
// DONE: handle recv errors
// TODO: error handling malloc
// TODO: error handling threads

void* scoreboardTimerThread() {
  SDL_Event new_event;
  new_event.type = Event_ScoreBoard;

  while (1) {
    sleep(5);
    SDL_PushEvent(&new_event);
  }
  return (NULL);
}

void* inactivityThread(void* args) {
  entity* character = args;

  while (1) {
    sleep(5);
    SDL_Event new_event;
    new_event.type = Event_Inactivity;
    new_event.user.data1 = character;
    SDL_PushEvent(&new_event);
  }
  return (NULL);
}

void* clientThread(void* args) {
  event_message msg;
  int err_rcv;
  event_struct* event_data;
  characters* chars = args;
  SDL_Event new_event;
  time_t timeout = time(NULL);
  int n_pacman_m = 0, n_monster_m = 0, valid = 0;
  pthread_t thread_pacman, thread_monster;

  if (DEBUG)
    printf("just connected to the server\n");

  pthread_create(&thread_pacman, NULL, inactivityThread, chars->pacman);
  pthread_create(&thread_monster, NULL, inactivityThread, chars->monster);

  chars->pacman->u_details->pacman_thread = thread_pacman;
  chars->pacman->u_details->monster_thread = thread_monster;

  while ((err_rcv = recv(chars->pacman->u_details->client_socket, &msg,
                         sizeof(event_message), 0)) > 0) {
    if (msg.dir >= 0 && msg.dir <= 4) {
      if ((time(NULL) - timeout) > 0) {
        n_pacman_m = 0;
        n_monster_m = 0;
        timeout = time(NULL);
      }
      if (msg.type) {
        if (n_pacman_m < 2) {
          valid = 1;
          pthread_cancel(thread_pacman);
          pthread_create(&thread_pacman, NULL, inactivityThread, chars->pacman);
          n_pacman_m++;
        } else {
          valid = 0;
        }
      } else {
        if (n_monster_m < 2) {
          valid = 1;
          pthread_cancel(thread_monster);
          pthread_create(&thread_monster, NULL, inactivityThread,
                         chars->monster);
          n_monster_m++;
        } else {
          valid = 0;
        }
      }

      if (valid) {
        if (DEBUG)
          printf("received %d type: %d dir: %d\n", err_rcv, msg.type, msg.dir);
        event_data = malloc(sizeof(event_struct));
        event_data->dir = msg.dir;
        event_data->client = chars->pacman;
        event_data->type = msg.type;

        SDL_zero(new_event);
        new_event.type = Event_Move;
        new_event.user.data1 = event_data;
        SDL_PushEvent(&new_event);
      }
    }
  }
  if (err_rcv == -1) {
    printf("ERROR: recv move disconnecting client...\n");
  }

  pthread_cancel(thread_pacman);
  pthread_cancel(thread_monster);

  SDL_zero(new_event);
  new_event.type = Event_Disconnect;
  new_event.user.data1 = chars->pacman;
  SDL_PushEvent(&new_event);
  free(chars);
  return (NULL);
}

void* connectionThread() {
  int server_socket, client_socket, err, r, g, b;
  struct sockaddr_in server_local_addr, client_addr;
  socklen_t size_addr = sizeof(client_addr);
  user_details* client;
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
      printf("ERROR: accept new connection, ignoring connection...\n");
    } else {
      if (DEBUG)
        printf("accepted connection \n");

      if (recv(client_socket, &r, sizeof(int), 0) == -1) {
        printf("ERROR: recv r code, rejecting connection...\n");
        r = -1;
      }
      if (recv(client_socket, &g, sizeof(int), 0) == -1) {
        printf("ERROR: recv g code, rejecting connection...\n");
        g = -1;
      }
      if (recv(client_socket, &b, sizeof(int), 0) == -1) {
        printf("ERROR: recv b code, rejecting connection...\n");
        b = -1;
      }

      if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        coords board_size;

        board_size.x = -2;
        board_size.y = -2;

        if (send(client_socket, &board_size, sizeof(coords), 0) == -1) {
          printf("ERROR: send rejection failed, ignoring client...\n");
        }
        printf(
            "A new user tried to connect with an invalid code RGB not "
            "valid!\n");
        close(client_socket);
      } else {
        SDL_zero(new_event);
        new_event.type = Event_NewUser;
        client = get_newUser(client_socket, r, g, b);
        new_event.user.data1 = client;
        SDL_PushEvent(&new_event);
      }
    }
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
                    int* n_fruits,
                    int n_lines,
                    int n_cols,
                    int* fruit_cap,
                    int max_clients) {
  entity* new_entity;
  int random_idx;
  pthread_t thread_id;
  coords board_size;
  int new_fruit_cap, new_fruits;

  new_fruit_cap = (*n_clients) * 2;
  new_fruits = new_fruit_cap - *fruit_cap;

  if (*n_clients >= max_clients) {
    board_size.x = -1;
    board_size.y = -1;
    if (send(new_client_details->client_socket, &board_size, sizeof(coords),
             0) == -1) {
      printf("ERROR: sending board size, discarding client\n");
      close(new_client_details->client_socket);
      free(new_client_details);
    }
    close(new_client_details->client_socket);
    free(new_client_details);
  } else {
    board_size.x = n_lines;
    board_size.y = n_cols;
    if (send(new_client_details->client_socket, &board_size, sizeof(coords),
             0) == -1) {
      printf("ERROR: sending board size, discarding client\n");
      close(new_client_details->client_socket);
      free(new_client_details);
    } else if (send(new_client_details->client_socket, &max_clients,
                    sizeof(int), 0) == -1) {
      printf("ERROR: sending max clients, discarding client\n");
      close(new_client_details->client_socket);
      free(new_client_details);
    } else {
      *fruit_cap = new_fruit_cap;
      for (int i = 0; i < new_fruits; i++) {
        respawn_fruit(n_free_spaces, free_spaces, n_fruits, fruits, *fruit_cap);
        send_Fruit(fruits, *n_fruits, pacmans, *n_clients);
      }

      characters* chars = malloc(sizeof(characters));

      // get random free space for entering pacman
      random_idx = random() % *n_free_spaces;
      new_entity = free_spaces[random_idx];
      new_entity->type = 2;
      new_entity->idx = *n_clients;
      new_entity->u_details = new_client_details;
      pacmans[*n_clients] = new_entity;
      if ((*n_free_spaces - 1) != random_idx) {
        free_spaces[random_idx] = free_spaces[*n_free_spaces - 1];
        free_spaces[random_idx]->idx = random_idx;
      }
      (*n_free_spaces)--;
      paint_pacman(pacmans[*n_clients]->column, pacmans[*n_clients]->line,
                   pacmans[*n_clients]->u_details->r,
                   pacmans[*n_clients]->u_details->g,
                   pacmans[*n_clients]->u_details->b);

      // get random free space for entering monster
      random_idx = random() % *n_free_spaces;
      new_entity = free_spaces[random_idx];
      new_entity->type = 3;
      new_entity->idx = *n_clients;
      new_entity->u_details = new_client_details;
      monsters[*n_clients] = new_entity;
      // move last position
      if ((*n_free_spaces - 1) != random_idx) {
        free_spaces[random_idx] = free_spaces[*n_free_spaces - 1];
        free_spaces[random_idx]->idx = random_idx;
      }
      (*n_free_spaces)--;
      paint_monster(monsters[*n_clients]->column, monsters[*n_clients]->line,
                    monsters[*n_clients]->u_details->r,
                    monsters[*n_clients]->u_details->g,
                    monsters[*n_clients]->u_details->b);

      chars->pacman = pacmans[*n_clients];
      chars->monster = monsters[*n_clients];
      pthread_create(&thread_id, NULL, clientThread, chars);
      chars->pacman->u_details->client_thread = thread_id;

      if (*n_clients > 0)
        send_AllClients(*n_clients, pacmans, monsters);
      if (n_bricks > 0)
        send_AllBricks(pacmans[*n_clients], bricks, n_bricks);
      if (n_fruits > 0)
        send_AllFruits(pacmans[*n_clients], fruits, *n_fruits);

      (*n_clients)++;

      send_NewClient(*n_clients, pacmans, monsters);
    }
  }
}

void handle_Disconnect(entity** pacmans,
                       entity** monsters,
                       entity** free_spaces,
                       entity** fruits,
                       int n_lines,
                       int n_cols,
                       int client_id,
                       int* n_free_spaces,
                       int* n_clients,
                       int* n_fruits,
                       int* fruit_cap) {
  entity* new_entity;
  int n_fruits_remove;
  coords clear_space;

  *fruit_cap = (*n_clients - 2) * 2;
  if (*fruit_cap < 0) {
    *fruit_cap = 0;
  }

  if (DEBUG)
    printf("disconnecting client %d\n", client_id);
  // get new free space with coords of leaving pacman
  new_entity = pacmans[client_id];
  new_entity->type = -1;
  new_entity->idx = *n_free_spaces;
  close(new_entity->u_details->client_socket);
  free(new_entity->u_details);
  free_spaces[*n_free_spaces] = new_entity;
  clear_place(free_spaces[*n_free_spaces]->column,
              free_spaces[*n_free_spaces]->line);
  (*n_free_spaces)++;

  // get new free space with coords of leaving monster
  new_entity = monsters[client_id];
  new_entity->type = -1;
  new_entity->idx = *n_free_spaces;
  free_spaces[*n_free_spaces] = new_entity;
  clear_place(free_spaces[*n_free_spaces]->column,
              free_spaces[*n_free_spaces]->line);
  (*n_free_spaces)++;

  // move last member of array to leaving idx
  if ((*n_clients) - 1 != client_id) {
    pacmans[client_id] = pacmans[*n_clients - 1];
    pacmans[client_id]->idx = client_id;
    monsters[client_id] = monsters[*n_clients - 1];
    monsters[client_id]->idx = client_id;
  }

  (*n_clients)--;

  if (*n_clients == 1) {
    pacmans[*n_clients - 1]->u_details->score = 0;
  }

  send_Disconnect(client_id, *n_clients, pacmans);
  n_fruits_remove = *n_fruits - *fruit_cap;
  printf("fruits %d %d %d\n", n_fruits_remove, *n_fruits, *fruit_cap);

  for (int i = 0; i < n_fruits_remove; i++) {
    free_spaces[*n_free_spaces] = fruits[*n_fruits - 1];
    free_spaces[*n_free_spaces]->idx = *n_free_spaces;
    free_spaces[*n_free_spaces]->type = -1;
    clear_place(free_spaces[*n_free_spaces]->column,
                free_spaces[*n_free_spaces]->line);
    clear_space.x = free_spaces[*n_free_spaces]->line;
    clear_space.y = free_spaces[*n_free_spaces]->column;
    send_Clear(pacmans, *n_clients, &clear_space);
    (*n_fruits)--;
    (*n_free_spaces)++;
  }
}

void handle_Inactivity(entity* character,
                       entity** free_spaces,
                       entity** pacmans,
                       entity** monsters,
                       entity*** board,
                       int n_free_spaces,
                       int n_clients) {
  int rand_idx = random() % n_free_spaces;
  int line = free_spaces[rand_idx]->line;
  int column = free_spaces[rand_idx]->column;
  int updated[2];

  free_spaces[rand_idx]->line = character->line;
  free_spaces[rand_idx]->column = character->column;
  character->line = line;
  character->column = column;

  board[free_spaces[rand_idx]->line][free_spaces[rand_idx]->column] =
      free_spaces[rand_idx];
  board[character->line][character->column] = character;

  clear_place(free_spaces[rand_idx]->column, free_spaces[rand_idx]->line);
  if (character->type == 2) {
    paint_pacman(character->column, character->line, character->u_details->r,
                 character->u_details->g, character->u_details->b);
    updated[0] = 1;
    updated[1] = character->idx;
  } else if (character->type == 3) {
    paint_monster(character->column, character->line, character->u_details->r,
                  character->u_details->g, character->u_details->b);
    updated[0] = 0;
    updated[1] = character->idx;
  }

  send_Move(updated, pacmans, monsters, n_clients);
}

void handle_ScoreBoard(int n_clients, entity** pacmans) {
  for (int i = 0; i < n_clients; i++) {
    send_ScoreBoard(pacmans, i, n_clients);
  }
}

int main(int argc, char* argv[]) {
  int done = 0, n_lines = 0, n_cols = 0;
  SDL_Event event;
  pthread_t connection_thread, scoreboard_thread;
  entity **bricks, **free_spaces;
  entity*** board;
  int n_clients = 0, n_bricks = 0, n_free_spaces = 0, n_fruits = 0,
      fruit_cap = 0, max_clients = 0, max_fruits = 0;
  char c;

  Event_Move = SDL_RegisterEvents(1);
  Event_NewUser = SDL_RegisterEvents(1);
  Event_Disconnect = SDL_RegisterEvents(1);
  Event_Inactivity = SDL_RegisterEvents(1);
  Event_ScoreBoard = SDL_RegisterEvents(1);

  if (argc != 2) {  // number of args verification
    printf("Incorrect number of arguments\n");
    exit(1);
  }

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
  max_clients = floor((n_free_spaces + 2) / 4);
  printf("Max Clients: %d\n", max_clients);
  max_fruits = (max_clients - 1) * 2;
  printf("Max fruits: %d\n", max_fruits);

  entity *pacmans[max_clients], *monsters[max_clients], *fruits[max_fruits];
  int aux1 = 0;
  int aux2 = 0;

  for (int i = 0; i < n_lines; i++) {
    for (int a = 0; a < n_cols; a++) {
      if ((board[i][a])->type == 6) {  //é brick

        bricks[aux1] = board[i][a];
        aux1++;
      } else if (board[i][a]->type == -1) {  //é free space
        free_spaces[aux2] = board[i][a];
        aux2++;
      }
    }
  }
  create_board_window(n_cols, n_lines);

  for (int i = 0; i < n_lines; i++) {
    for (int a = 0; a < n_cols; a++) {
      if ((board[i][a]) != NULL) {
        if ((board[i][a])->type == 6) {
          paint_brick(a, i);
        }
      }
    }
  }

  pthread_create(&connection_thread, NULL, connectionThread, NULL);
  pthread_create(&scoreboard_thread, NULL, scoreboardTimerThread, NULL);

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      } else if (event.type == Event_Move) {
        handle_mov_init(event.user.data1, board, n_lines, n_cols, pacmans,
                        monsters, fruits, free_spaces, &n_fruits,
                        &n_free_spaces, n_clients);
      } else if (event.type == Event_NewUser) {
        user_details* client;
        client = event.user.data1;
        if (DEBUG)
          printf("fd %d\n", client->client_socket);
        handle_NewUser(client, pacmans, monsters, free_spaces, bricks, fruits,
                       &n_clients, &n_free_spaces, n_bricks, &n_fruits, n_lines,
                       n_cols, &fruit_cap, max_clients);
      } else if (event.type == Event_Disconnect) {
        entity* client;
        client = event.user.data1;
        handle_Disconnect(pacmans, monsters, free_spaces, fruits, n_lines,
                          n_cols, client->idx, &n_free_spaces, &n_clients,
                          &n_fruits, &fruit_cap);
      } else if (event.type == Event_Inactivity) {
        entity* character;
        character = event.user.data1;
        handle_Inactivity(character, free_spaces, pacmans, monsters, board,
                          n_free_spaces, n_clients);
      } else if (event.type == Event_RespawnFruit) {
        respawn_fruit(&n_free_spaces, free_spaces, &n_fruits, fruits,
                      fruit_cap);
        send_Fruit(fruits, n_fruits, pacmans, n_clients);
      } else if (event.type == Event_ScoreBoard) {
        handle_ScoreBoard(n_clients, pacmans);
      }
    }
  }

  free_memory(board, n_lines, n_cols, free_spaces, pacmans, bricks, n_clients);
  pthread_cancel(connection_thread);
  pthread_cancel(scoreboard_thread);
}
