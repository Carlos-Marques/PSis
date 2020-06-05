#include "server_comms.h"

void handle_send_error(entity* client) {
  SDL_Event new_event;
  if(pthread_cancel(client->u_details->client_thread)){
    perror("ERROR CANCELING THREAD!\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_cancel(client->u_details->pacman_thread)){
    perror("ERROR CANCELING THREAD!\n");
    exit(EXIT_FAILURE);
  }
  if(pthread_cancel(client->u_details->monster_thread)){
    perror("ERROR CANCELING THREAD!\n");
    exit(EXIT_FAILURE);
  }
  SDL_zero(new_event);
  new_event.type = Event_Disconnect;
  new_event.user.data1 = client;
  SDL_PushEvent(&new_event);
}

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

    if (send(socket, &message_type, sizeof(int), 0) == -1) {
      printf("ERROR: sending message type %d, disconnecting client %d\n",
             message_type, i);
      handle_send_error(pacmans[i]);
    } else if (send(socket, &client_color, sizeof(rgb), 0) == -1) {
      printf("ERROR: sending client color, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    } else if (send(socket, &pacman_coords, sizeof(pacman_coords), 0) == -1) {
      printf("ERROR: sending pacman coords, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    } else if (send(socket, &monster_coords, sizeof(monster_coords), 0) == -1) {
      printf("ERROR: sending monster coords, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    }
  }
}

void send_Disconnect(int client_id, int n_clients, entity** pacmans) {
  int message_type = 1, socket;
  for (int i = 0; i < n_clients; i++) {
    socket = pacmans[i]->u_details->client_socket;

    if (send(socket, &message_type, sizeof(int), 0) == -1) {
      printf("ERROR: sending message type %d, disconnecting client %d\n",
             message_type, i);
      handle_send_error(pacmans[i]);
    } else if (send(socket, &client_id, sizeof(client_id), 0) == -1) {
      printf("ERROR: sending client id, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    }
  }
}

void send_AllClients(int n_clients, entity** pacmans, entity** monsters) {
  int message_type = 2, socket;
  coords pacman_coords, monster_coords;
  rgb client_color;

  if (DEBUG)
    printf("Sending all clients: %d\n", n_clients);

  socket = pacmans[n_clients]->u_details->client_socket;
  if (send(socket, &message_type, sizeof(int), 0) == -1) {
    printf("ERROR: sending message type %d, disconnecting client %d\n",
           message_type, n_clients);
    handle_send_error(pacmans[n_clients]);
  } else if (send(socket, &n_clients, sizeof(int), 0) == -1) {
    printf("ERROR: sending number of clients, disconnecting client %d\n",
           n_clients);
    handle_send_error(pacmans[n_clients]);
  } else {
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

      if (send(socket, &client_color, sizeof(rgb), 0) == -1) {
        printf("ERROR: sending client color, disconnecting client %d\n", i);
        handle_send_error(pacmans[i]);
      } else if (send(socket, &pacman_coords, sizeof(coords), 0) == -1) {
        printf("ERROR: sending pacman coords, disconnecting client %d\n", i);
        handle_send_error(pacmans[i]);
      } else if (send(socket, &monster_coords, sizeof(coords), 0) == -1) {
        printf("ERROR: sending monster coords, disconnecting client %d\n", i);
        handle_send_error(pacmans[i]);
      }
    }
  }
}

void send_AllBricks(entity* client, entity** bricks, int n_bricks) {
  int message_type = 3;
  coords brick;

  if (DEBUG)
    printf("Sending all bricks\n");

  if (send(client->u_details->client_socket, &message_type, sizeof(int), 0) ==
      -1) {
    printf("ERROR: sending message type %d, disconnecting client %d\n",
           message_type, client->idx);
    handle_send_error(client);
  } else if (send(client->u_details->client_socket, &n_bricks, sizeof(int),
                  0) == -1) {
    printf("ERROR: sending number of bricks, disconnecting client %d\n",
           client->idx);
    handle_send_error(client);
  } else {
    for (int i = 0; i < n_bricks; i++) {
      brick.x = bricks[i]->line;
      brick.y = bricks[i]->column;

      if (DEBUG)
        printf("brick coords: %d %d\n", brick.x, brick.y);

      if (send(client->u_details->client_socket, &brick, sizeof(coords), 0) ==
          -1) {
        printf("ERROR: sending brick coords, disconnecting client %d\n",
               client->idx);
        handle_send_error(client);
      }
    }
  }
}

void send_AllFruits(entity* client, entity** fruits, int n_fruits) {
  int message_type = 4;
  coords fruit;

  if (DEBUG)
    printf("Sending all fruits\n");

  if (send(client->u_details->client_socket, &message_type, sizeof(int), 0) ==
      -1) {
    printf("ERROR: sending message type %d, disconnecting client %d\n",
           message_type, client->idx);
    handle_send_error(client);
  } else if (send(client->u_details->client_socket, &n_fruits, sizeof(int),
                  0) == -1) {
    printf("ERROR: sending number of fruits, disconnecting client %d\n",
           client->idx);
    handle_send_error(client);
  } else {
    for (int i = 0; i < n_fruits; i++) {
      fruit.x = fruits[i]->line;
      fruit.y = fruits[i]->column;

      if (DEBUG)
        printf("fruits coords: %d %d\n", fruit.x, fruit.y);

      if (send(client->u_details->client_socket, &fruit, sizeof(coords), 0) ==
          -1) {
        printf("ERROR: sending fruit coords, disconnecting client %d\n",
               client->idx);
        handle_send_error(client);
      } else if (send(client->u_details->client_socket, &fruits[i]->type,
                      sizeof(int), 0) == -1) {
        printf("ERROR: sending fruit type, disconnecting client %d\n",
               client->idx);
        handle_send_error(client);
      }
    }
  }
}

void send_Move(int* updated,
               entity** pacmans,
               entity** monsters,
               int n_clients) {
  int message_type, i;
  coords updated_coords;

  if (updated[0] == 1) {
    message_type = 5;  // Move pacman
    updated_coords.x = pacmans[updated[1]]->line;
    updated_coords.y = pacmans[updated[1]]->column;
  } else if (updated[0] == 0) {  // Move monster
    message_type = 6;
    updated_coords.x = monsters[updated[1]]->line;
    updated_coords.y = monsters[updated[1]]->column;
  } else if (updated[0] == 2) {
    message_type = 7;  // Move superpacman
    updated_coords.x = pacmans[updated[1]]->line;
    updated_coords.y = pacmans[updated[1]]->column;
  } else if (updated[0] == 3) {
    message_type = 8;  // Update pacman
    updated_coords.x = pacmans[updated[1]]->line;
    updated_coords.y = pacmans[updated[1]]->column;
  } else if (updated[0] == 4) {  // Update monster
    message_type = 9;
    updated_coords.x = monsters[updated[1]]->line;
    updated_coords.y = monsters[updated[1]]->column;
  } else if (updated[0] == 5) {
    message_type = 10;  // Update pacman
    updated_coords.x = pacmans[updated[1]]->line;
    updated_coords.y = pacmans[updated[1]]->column;
  }

  for (i = 0; i < n_clients; i++) {
    if (send(pacmans[i]->u_details->client_socket, &message_type, sizeof(int),
             0) == -1) {
      printf("ERROR: sending message type %d, disconnecting client %d\n",
             message_type, i);
      handle_send_error(pacmans[i]);
    } else if (send(pacmans[i]->u_details->client_socket, &updated[1],
                    sizeof(int), 0) == -1) {
      printf("ERROR: sending updated index, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    } else if (send(pacmans[i]->u_details->client_socket, &updated_coords,
                    sizeof(coords), 0) == -1) {
      printf("ERROR: sending updated coords, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    }
  }
}

void send_Fruit(entity** fruits,
                int n_fruits,
                entity** pacmans,
                int n_clients) {
  int message_type;
  printf("Fruit type: %d\n", fruits[n_fruits - 1]->type);
  if (fruits[n_fruits - 1]->type) {
    message_type = 12;
    printf("Sending Lemon\n");
  } else {
    message_type = 11;
    printf("Sending Cherry\n");
  }
  coords new_fruit_coords;
  new_fruit_coords.x = fruits[n_fruits - 1]->line;
  new_fruit_coords.y = fruits[n_fruits - 1]->column;

  for (int i = 0; i < n_clients; i++) {
    if (send(pacmans[i]->u_details->client_socket, &message_type, sizeof(int),
             0) == -1) {
      printf("ERROR: sending message type %d, disconnecting client %d\n",
             message_type, i);
      handle_send_error(pacmans[i]);
    } else if (send(pacmans[i]->u_details->client_socket, &new_fruit_coords,
                    sizeof(coords), 0) == -1) {
      printf("ERROR: sending fruits coords, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    }
  }
}

void send_ScoreBoard(entity** pacmans, int sendTo, int n_users) {
  int message_type = 13;
  rgb color;
  scoreB u;

  if (send(pacmans[sendTo]->u_details->client_socket, &message_type,
           sizeof(int), 0) == -1) {
    printf("ERROR: sending message type %d, disconnecting client %d\n",
           message_type, sendTo);
    handle_send_error(pacmans[sendTo]);
  } else {
    for (int i = 0; i < n_users; i++) {
      color.r = pacmans[i]->u_details->r;
      color.g = pacmans[i]->u_details->g;
      color.b = pacmans[i]->u_details->b;

      u.id = pacmans[i]->idx;
      u.score = pacmans[i]->u_details->score;

      if (send(pacmans[sendTo]->u_details->client_socket, &color, sizeof(rgb),
               0) == -1) {
        printf("ERROR: sending client color, disconnecting client %d\n", i);
        handle_send_error(pacmans[i]);
      } else if (send(pacmans[sendTo]->u_details->client_socket, &u,
                      sizeof(scoreB), 0) == -1) {
        printf("ERROR: sending score, disconnecting client %d\n", i);
        handle_send_error(pacmans[i]);
      }
    }
  }
}

void send_Clear(entity** pacmans, int n_clients, coords* clear_space) {
  int message_type = 14;

  for (int i = 0; i < n_clients; i++) {
    if (send(pacmans[i]->u_details->client_socket, &message_type, sizeof(int),
             0) == -1) {
      printf("ERROR: sending message type %d, disconnecting client %d\n",
             message_type, i);
      handle_send_error(pacmans[i]);
    } else if (send(pacmans[i]->u_details->client_socket, clear_space,
                    sizeof(coords), 0) == -1) {
      printf("ERROR: sending clear space coords, disconnecting client %d\n", i);
      handle_send_error(pacmans[i]);
    }
  }
}
