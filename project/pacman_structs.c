#include "pacman_structs.h"

entity* get_newEntity(int x,
                      int y,
                      int type,
                      int idx,
                      user_details* u_details) {
  entity* ent = (entity*)malloc(sizeof(entity));

  ent->line = x;
  ent->column = y;
  ent->type = type;

  ent->idx = idx;

  ent->u_details = u_details;

  return ent;
}

user_details* get_newUser(int socket, int r, int g, int b) {
  user_details* u_details = (user_details*)malloc(sizeof(user_details));

  u_details->r = r;
  u_details->g = g;
  u_details->b = b;
  u_details->score = 0;
  u_details->client_socket = socket;

  return u_details;
}

void free_memory(entity*** board, int n_lin, int n_col, entity** free_spaces, entity** pacmans, entity** bricks, int n_clients) {
  
  for (int i = 0; i < n_clients; i++)
  {
    free(pacmans[i]->u_details);
    close(pacmans[i]->u_details->client_socket);
  }

  for (int i = 0; i < n_lin; i++) {
    for (int a = 0; a < n_col; a++) {
      free(board[i][a]);
    }
    free(board[i]);
  }
  free(board);
  free(free_spaces);
  free(bricks);
}
