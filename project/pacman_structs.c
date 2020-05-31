#include <stdlib.h>
#include "pacman.h"

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

user_details* get_newUser(int socket) {
  user_details* u_details = (user_details*)malloc(sizeof(user_details));

  u_details->r = random() % 255;
  u_details->g = random() % 255;
  u_details->b = random() % 255;
  u_details->score = 0;
  u_details->client_socket = socket;

  return u_details;
}

void free_board(entity**** ent, int n_lin, int n_col) {
  for (int i = 0; i < n_lin; i++) {
    for (int a = 0; a < n_col; a++) {
      free((*ent)[i][a]);
    }
    free((*ent)[i]);
  }
  free(*ent);
}
