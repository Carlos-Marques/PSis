#include "client.h"

void* serverThread(void* args) {
  server_args* server = args;
  int err_rcv, message_type;
  int n_clients = 0, max_clients = 0;
  coords board_size;

  if (recv(server->server_socket, &board_size, sizeof(coords), 0) == -1) {
    perror("ERROR");
    exit(EXIT_FAILURE);
  }
  if (board_size.x == -1 && board_size.y == -1) {
    printf("Server is full!\n");
    exit(1);
  } else if (board_size.x == -2 && board_size.y == -2) {
    printf("Invalid RGB code!\n");
    exit(1);
  }
  printf("board: %d %d\n", board_size.x, board_size.y);
  create_board_window(board_size.y, board_size.x);

  if (recv(server->server_socket, &max_clients, sizeof(int), 0) == -1) {
    perror("ERROR");
    exit(EXIT_FAILURE);
  }
  printf("max clients: %d\n", max_clients);

  client_data* clients[100];

  while ((err_rcv =
              recv(server->server_socket, &message_type, sizeof(int), 0)) > 0) {
    printf("message type: %d\n", message_type);
    if (message_type == 0) {
      rcv_NewClient(clients, server->server_socket, &n_clients);
      if (!server->ready) {
        server->pacman_coords = clients[n_clients - 1]->pacman_coords;
        server->ready = 1;
      }
    } else if (message_type == 1) {
      rcv_Disconnect(clients, server->server_socket, &n_clients);
    } else if (message_type == 2) {
      rcv_AllClients(clients, server->server_socket, &n_clients);
    } else if (message_type == 3) {
      rcv_AllBricks(server->server_socket);
    } else if (message_type == 4) {
      rcv_AllFruits(server->server_socket);
    } else if (message_type == 5) {
      rcv_MovePacman(server->server_socket, clients, 1, 0);
    } else if (message_type == 6) {
      rcv_MoveMonster(server->server_socket, clients, 1);
    } else if (message_type == 7) {
      rcv_MovePacman(server->server_socket, clients, 1, 1);
    } else if (message_type == 8) {
      rcv_MovePacman(server->server_socket, clients, 0, 0);
    } else if (message_type == 9) {
      rcv_MoveMonster(server->server_socket, clients, 0);
    } else if (message_type == 10) {
      rcv_MovePacman(server->server_socket, clients, 0, 1);
    } else if (message_type == 11) {
      rcv_Cherry(server->server_socket);
    } else if (message_type == 12) {
      rcv_Lemon(server->server_socket);
    } else if (message_type == 13) {
      rcv_ScoreBoard(server->server_socket, n_clients);
    } else if (message_type == 14) {
      rcv_Clear(server->server_socket);
    }
  }
  if (err_rcv == -1) {
    perror("ERROR");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n_clients; i++) {
    free(clients[i]->monster_coords);
    free(clients[i]->pacman_coords);
    free(clients[i]->rgb);
    free(clients[i]);
  }

  server->ready = 0;
  server->done = 1;

  return (NULL);
}

int main(int argc, char* argv[]) {
  struct sockaddr_in server_addr;
  int sock_fd, r, g, b;
  event_message move_msg;
  server_args server;
  time_t pacman_timer = time(NULL), monster_timer = time(NULL);
  int n_pacman_m = 0, n_monster_m = 0;
  int x = -1, y = -1;
  pthread_t thread_id;
  SDL_Event event;

  if (argc != 6) {
    printf("Incorrect number of arguments\n");
    exit(1);
  }

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

  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
    printf("Code RGB not valid!\n");
    exit(-1);
  }

  printf("connecting to %s %d\n", argv[1], server_addr.sin_port);

  if (-1 == connect(sock_fd, (const struct sockaddr*)&server_addr,
                    sizeof(server_addr))) {
    printf("Error connecting\n");
    exit(-1);
  }

  if (send(sock_fd, &r, sizeof(int), 0) == -1) {
    perror("ERROR\n");
    exit(EXIT_FAILURE);
  }
  if (send(sock_fd, &g, sizeof(int), 0) == -1) {
    perror("ERROR\n");
    exit(EXIT_FAILURE);
  }
  if (send(sock_fd, &b, sizeof(int), 0) == -1) {
    perror("ERROR\n");
    exit(EXIT_FAILURE);
  }

  server.server_socket = sock_fd;
  server.ready = 0;
  server.done = 0;

  if (pthread_create(&thread_id, NULL, serverThread, &server)) {
    perror("ERROR CREATING NEW THREAD!\n");
    exit(EXIT_FAILURE);
  }

  while (!server.done) {
    while (server.ready) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          server.done = SDL_TRUE;
          server.ready = 0;
          if (pthread_cancel(thread_id)) {
            perror("ERROR CANCELING THREAD!\n");
            exit(EXIT_FAILURE);
          }
        } else if (event.type == SDL_KEYDOWN) {
          move_msg.type = 0;
          if ((time(NULL) - monster_timer) > 0) {
            monster_timer = time(NULL);
            n_monster_m = 0;
          }
          if (n_monster_m < 2) {
            if (event.key.keysym.sym == SDLK_DOWN) {
              printf("down!\n");
              move_msg.dir = 0;
              n_monster_m++;

              if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                perror("ERROR\n");
                exit(EXIT_FAILURE);
              }
            } else if (event.key.keysym.sym == SDLK_UP) {
              printf("up!\n");
              move_msg.dir = 1;
              n_monster_m++;

              if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                perror("ERROR\n");
                exit(EXIT_FAILURE);
              }
            } else if (event.key.keysym.sym == SDLK_LEFT) {
              printf("left!\n");
              move_msg.dir = 2;
              n_monster_m++;

              if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                perror("ERROR\n");
                exit(EXIT_FAILURE);
              }
            } else if (event.key.keysym.sym == SDLK_RIGHT) {
              printf("right!\n");
              move_msg.dir = 3;
              n_monster_m++;

              if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                perror("ERROR\n");
                exit(EXIT_FAILURE);
              }
            }
          }
        } else if (event.type == SDL_MOUSEMOTION) {
          move_msg.type = 1;
          int x_new, y_new;
          // this fucntion return the place cwher the mouse cursor is
          get_board_place(event.motion.x, event.motion.y, &x_new, &y_new);
          // if the mluse moved toi anothe place
          if ((x_new != x) || (y_new != y)) {
            if ((time(NULL) - pacman_timer) > 0) {
              pacman_timer = time(NULL);
              n_pacman_m = 0;
            }
            if (n_pacman_m < 2) {
              printf("x-new = %d \n", x_new);
              printf("y-new = %d \n", y_new);
              x = x_new;
              y = y_new;
              int delta_x = x_new - server.pacman_coords->y;
              int delta_y = y_new - server.pacman_coords->x;

              if (delta_x == 1 && delta_y == 0) {
                printf("right! %d\n", x_new);
                move_msg.dir = 3;
                n_pacman_m++;

                if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                  perror("ERROR\n");
                  exit(EXIT_FAILURE);
                }
              } else if (delta_x == -1 && delta_y == 0) {
                printf("left!\n");
                move_msg.dir = 2;
                n_pacman_m++;

                if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                  perror("ERROR\n");
                  exit(EXIT_FAILURE);
                }
              } else if (delta_y == 1 && delta_x == 0) {
                printf("down!\n");
                move_msg.dir = 0;
                n_pacman_m++;

                if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                  perror("ERROR\n");
                  exit(EXIT_FAILURE);
                }
              } else if (delta_y == -1 && delta_x == 0) {
                printf("up!\n");
                move_msg.dir = 1;
                n_pacman_m++;

                if (send(sock_fd, &move_msg, sizeof(event_message), 0) == -1) {
                  perror("ERROR\n");
                  exit(EXIT_FAILURE);
                }
              }
            }
          }
        }
      }
    }
  }
}
