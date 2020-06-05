#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct message {
  int character;  // 2 pacman 3 monster
  int x;
  int y;
} message;

typedef struct board_dim_message {
  int x;
  int y;
} board_dim_message;

typedef struct user_details {
  int r;
  int g;
  int b;

  int score;
  int client_socket;

  pthread_t client_thread;
  pthread_t pacman_thread;
  pthread_t monster_thread;
} user_details;

typedef struct entity {
  int line;
  int column;

  int type;  // 0:Cherry | 1:Lemon | 2:Pacman | 3:Monster |
             // (MAXINT-1:Charge_Pacman) | MAXINT:Wall |
  int idx;

  user_details* u_details;

} entity;

//-------------------FUNCTIONS-------------------

entity* get_newEntity(int x, int y, int type, int idx, user_details* u_details);

user_details* get_newUser(int socket, int r, int g, int b);

void free_memory(entity*** board,
                 int n_lin,
                 int n_col,
                 entity** free_spaces,
                 entity** pacmans,
                 entity** bricks,
                 int n_clients);
