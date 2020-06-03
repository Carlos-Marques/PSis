#include <stdlib.h>

typedef struct message{
  int character; // 2 pacman 3 monster
  int x;
  int y;
} message;

typedef struct board_dim_message{
  int x;
  int y;
} board_dim_message;

typedef struct user_details{
  int r;
  int g;
  int b;

  int score;

} user_details;

typedef struct entity{
  int line;
  int column;

  int type; //-1:free space | 0:Cherry | 1:Lemon | 2:Pacman | 3:Monster | (4 e 5:Charge_Pacman) | 6:Wall | 
  int idx;

  user_details* u_details;

} entity;


//-------------------FUNCTIONS-------------------

entity* get_newEntity(int x, int y, int type);

void free_board(entity**** ent, int n_lin, int n_col);