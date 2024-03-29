#include "pacman_structs.h"

entity* get_newEntity(int x,
                      int y,
                      int type,
                      int idx,
                      user_details* u_details) {
  entity* ent = (entity*)malloc(sizeof(entity));
  if (ent == NULL) {
    perror("ERROR IN MALLOC!\n");
    exit(EXIT_FAILURE);
  }

  ent->line = x;
  ent->column = y;
  ent->type = type;

  ent->idx = idx;

  ent->u_details = u_details;

  return ent;
}

user_details* get_newUser(int socket, int r, int g, int b) {
  user_details* u_details = (user_details*)malloc(sizeof(user_details));
  if (u_details == NULL) {
    perror("ERROR IN MALLOC!\n");
    exit(EXIT_FAILURE);
  }

  u_details->r = r;
  u_details->g = g;
  u_details->b = b;
  u_details->score = 0;
  u_details->client_socket = socket;

  return u_details;
}

void free_memory(entity*** board,
                 int n_lin,
                 int n_col,
                 entity** free_spaces,
                 entity** pacmans,
                 entity** bricks,
                 int n_clients, activeFruitThread* head_activeFruitThread, activeFruitThread* tail_activeFruitThread) {
  for (int i = 0; i < n_clients; i++) {
    
    free(pacmans[i]->u_details);
    
    if(close(pacmans[i]->u_details->client_socket)){
      perror("ERROR IN CLOSING SOCKET!\n");
      exit(EXIT_FAILURE);
    }
    if(pthread_cancel(pacmans[i]->u_details->pacman_thread)){
      perror("ERROR CANCELING THREAD!\n");
      exit(EXIT_FAILURE);
    }
    if(pthread_cancel(pacmans[i]->u_details->monster_thread)){
      perror("ERROR CANCELING THREAD!\n");
      exit(EXIT_FAILURE);
    }
    if(pthread_cancel(pacmans[i]->u_details->client_thread)){
      perror("ERROR CANCELING THREAD!\n");
      exit(EXIT_FAILURE);
    }
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

  //close ongoing fruit respawn threads
  if(head_activeFruitThread != NULL && tail_activeFruitThread != NULL){

    while ( head_activeFruitThread != NULL )
    {
      if (pthread_cancel(head_activeFruitThread->thread_id)) {
        perror("ERROR CANCELING THREAD!\n");
        exit(EXIT_FAILURE);
      }
      tail_activeFruitThread = head_activeFruitThread;
      head_activeFruitThread = head_activeFruitThread->next;

      free(tail_activeFruitThread);
    }
  }
}
