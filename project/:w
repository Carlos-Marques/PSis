#include "server_comms.h"

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

void send_Disconnect(int client_id, int n_clients, entity** pacmans) {
  int message_type = 1, socket;
  for (int i = 0; i < n_clients; i++) {
    socket = pacmans[i]->u_details->client_socket;
    send(socket, &message_type, sizeof(int), 0);
    send(socket, &client_id, sizeof(client_id), 0);
  }
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

void send_Move(int* updated,
               entity** pacmans,
               entity** monsters,
               int n_clients) {
  int message_type, i, j;
  coords updated_coords;
  if (updated[0]) {
    message_type = 5;  // Updated pacman
    updated_coords.x = pacmans[updated[1]]->line;
    updated_coords.y = pacmans[updated[1]]->column;
  } else {
    message_type = 6;
    updated_coords.x = monsters[updated[1]]->line;
    updated_coords.y = monsters[updated[1]]->column;
  }

  for (i = 0; i < n_clients; i++) {
    send(pacmans[i]->u_details->client_socket, &message_type, sizeof(int), 0);
    send(pacmans[i]->u_details->client_socket, &updated[1], sizeof(int), 0);
    send(pacmans[i]->u_details->client_socket, &updated_coords, sizeof(coords),
         0);
  }
}
