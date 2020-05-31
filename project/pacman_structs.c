#include "pacman.h"
#include <stdlib.h>

entity* get_newEntity(int x, int y, int type){

    entity* ent = (entity *)malloc(sizeof(entity));
    
    ent->line = x;
    ent->column = y;
    ent->type = type;

    ent->idx = 0;

    ent->u_details = NULL;

    return ent;
}

void free_board(entity**** ent, int n_lin, int n_col){

    for (int i = 0; i < n_lin; i++)
    {
        for (int a = 0; a < n_col; a++)
        {
            free((*ent)[i][a]);
        }
        free((*ent)[i]);
    }
    free(*ent);

}