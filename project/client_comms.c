#include "client_comms.h"

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
void rcv_MovePacman(int server_socket, client_data** clients) {
  int updated_idx;
  coords updated_coords;

  recv(server_socket, &updated_idx, sizeof(int), 0);
  recv(server_socket, &updated_coords, sizeof(coords), 0);

  clear_place(clients[updated_idx]->pacman_coords->y,
              clients[updated_idx]->pacman_coords->x);
  clients[updated_idx]->pacman_coords->x = updated_coords.x;
  clients[updated_idx]->pacman_coords->y = updated_coords.y;
  paint_pacman(clients[updated_idx]->pacman_coords->y,
               clients[updated_idx]->pacman_coords->x,
               clients[updated_idx]->rgb->r, clients[updated_idx]->rgb->g,
               clients[updated_idx]->rgb->b);
}

void rcv_MoveMonster(int server_socket, client_data** clients) {
  int updated_idx;
  coords updated_coords;

  recv(server_socket, &updated_idx, sizeof(int), 0);
  recv(server_socket, &updated_coords, sizeof(coords), 0);

  clear_place(clients[updated_idx]->monster_coords->y,
              clients[updated_idx]->monster_coords->x);
  clients[updated_idx]->monster_coords->x = updated_coords.x;
  clients[updated_idx]->monster_coords->y = updated_coords.y;
  paint_monster(clients[updated_idx]->monster_coords->y,
                clients[updated_idx]->monster_coords->x,
                clients[updated_idx]->rgb->r, clients[updated_idx]->rgb->g,
                clients[updated_idx]->rgb->b);
}
