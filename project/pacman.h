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

  int type; //0:Cherry | 1:Lemon | 2:Pacman | 3:Monster | (MAXINT-1:Charge_Pacman) | MAXINT:Wall | 
  int idx;

  user_details* u_details;

} entity;


//-------------------FUNCTIONS-------------------

entity* get_newEntity(int x, int y, int type);

void free_board(entity**** ent, int n_lin, int n_col);