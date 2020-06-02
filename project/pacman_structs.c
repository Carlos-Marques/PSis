#include "pacman.h"
#include <stdlib.h>
#include <time.h>

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

int* handle_mov(int idx, int dir, int type, entity**** board_map, int n_lin, int n_col, entity*** pacmans, entity*** monsters, entity*** fruits, entity*** free_space_list, int* fruit_counter, int* free_space_counter){

    int random, destination_line, destination_column, aux, bounce = 0;
    entity* ent, aux_ent;
    int updates[6];

    if(type == 2 || type == 4){ //pacman

        ent = (*pacmans)[idx];
    }
    else if(type == 3){         //monster

        ent = (*monsters)[idx];
    }


    if(dir == 0){           //down

        destination_line = ent->line + 1;
        destination_column = ent->column;

        if(destination_line > n_lin){   //out of window - bounce

            if(destination_line - 2 < 0){ //bounced out of window - double bounce
                destination_line -= 1;
            }
            else if((*board_map)[destination_line - 2][destination_column]->type == 6){ //bounced into wall - double bounce
                destination_line -= 1;
            }
            else{
                destination_line -= 2;  //bounce
            }
        }
        else if((*board_map)[destination_line][destination_column]->type == 6){    //into wall - bounce

            if(destination_line - 2 < 0){ //bounced out of window - double bounce
                destination_line -= 1;
            }
            else if((*board_map)[destination_line - 2][destination_column]->type == 6){ //bounced into wall - double bounce
                destination_line -= 1;
            }
            else{
                destination_line -= 2;  //bounce
            }
        }
    }
    else if(dir == 1){      //up

        destination_line = ent->line - 1;
        destination_column = ent->column;

        if(destination_line < 0){   //out of window - bounce

            if(destination_line + 2 > n_lin){ //bounced out of window - double bounce
                destination_line += 1;
            }
            else if((*board_map)[destination_line + 2][destination_column]->type == 6){ //bounced into wall - double bounce
                destination_line += 1;
            }
            else{
                destination_line += 2;  //bounce
            }
        }
        else if((*board_map)[destination_line][destination_column]->type == 6){    //into wall - bounce

            if(destination_line + 2 > n_lin){ //bounced out of window - double bounce
                destination_line += 1;
            }
            else if((*board_map)[destination_line + 2][destination_column]->type == 6){ //bounced into wall - double bounce
                destination_line += 1;
            }
            else{
                destination_line += 2;
            }
        }
    }
    else if(dir == 2){      //left

        destination_line = ent->line;
        destination_column = ent->column - 1;

        if(destination_column < 0){   //out of window - bounce

            if(destination_column + 2 > n_col){ //bounced out of window - double bounce
                destination_column += 1;
            }
            else if((*board_map)[destination_line][destination_column + 2]->type == 6){ //bounced into wall - double bounce
                destination_column += 1;
            }
            else{
                destination_column += 2;  //bounce
            }
        }
        else if((*board_map)[destination_line][destination_column]->type == 6){    //into wall - bounce

            if(destination_column + 2 > n_col){ //bounced out of window - double bounce
                destination_column += 1;
            }
            else if((*board_map)[destination_line][destination_column + 2]->type == 6){ //bounced into wall - double bounce
                destination_column += 1;
            }
            else{
                destination_column += 2;  //bounce
            }
        }
    }
    else if(dir == 3){      //right

        destination_line = ent->line;
        destination_column = ent->column + 1;

        if(destination_column > n_col){   //out of window - bounce

            if(destination_column - 2 < 0){ //bounced out of window - double bounce
                destination_column -= 1;
            }
            else if((*board_map)[destination_line][destination_column - 2]->type == 6){ //bounced into wall - double bounce
                destination_column -= 1;
            }
            else{
                destination_column -= 2;  //bounce
            }
        }
        else if((*board_map)[destination_line][destination_column]->type == 6){    //into wall - bounce

            if(destination_column - 2 < 0){ //bounced out of window - double bounce
                destination_column -= 1;
            }
            else if((*board_map)[destination_line][destination_column - 2]->type == 6){ //bounced into wall - double bounce
                destination_column -= 1;
            }
            else{
                destination_column -= 2;  //bounce
            }
        }
    }

    if(destination_line == ent->line && destination_column == ent->column){ //double bounce, no updates in SDL
        updates[0] = -2;     //vazio
    }
    else{

        if((*board_map)[destination_line][destination_column]->type == -1){ //entity mooove to free space

            //actualizar e meter a estrutura de free space no sitio onde estava antes o bicho
            (*board_map)[destination_line][destination_column]->line = ent->line;
            (*board_map)[destination_line][destination_column]->column = ent->column;

            (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

            updates[2] = (*board_map)[ent->line][ent->column]->type;
            updates[3] = (*board_map)[ent->line][ent->column]->idx;

            //actualizar e meter a estrutura do bicho no destino
            ent->line = destination_line;
            ent->column = destination_column;

            (*board_map)[destination_line][destination_column] = ent;

            updates[0] = ent->type;
            updates[1] = ent->idx;

            updates[4] = -2;    //vazio

        }
        //same types
        else if ( ent->type == (*board_map)[destination_line][destination_column]->type || ( (ent->type == 2 || ent->type == 4 || ent->type == 5) && ((*board_map)[destination_line][destination_column]->type == 2 || (*board_map)[destination_line][destination_column]->type == 4 || (*board_map)[destination_line][destination_column]->type == 5) ) ) //second condition for various types of pacman
        {
            /* troca */
            //actualizar e meter a estrutura de free space no sitio onde estava antes o bicho
            (*board_map)[destination_line][destination_column]->line = ent->line;
            (*board_map)[destination_line][destination_column]->column = ent->column;

            (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

            updates[2] = (*board_map)[ent->line][ent->column]->type;
            updates[3] = (*board_map)[ent->line][ent->column]->idx;

            //actualizar e meter a estrutura do bicho no destino
            ent->line = destination_line;
            ent->column = destination_column;

            (*board_map)[destination_line][destination_column] = ent;

            updates[0] = ent->type;
            updates[1] = ent->idx;

            updates[4] = -2;
        }
        else if (ent->type > (*board_map)[destination_line][destination_column]->type)
        {
            //verificar se é do mesmo user -> troca
            if(ent->idx == (*board_map)[destination_line][destination_column]->idx){

                //actualizar e meter a estrutura de monstro/pacman do mesmo user no sitio onde estava antes o bicho
                (*board_map)[destination_line][destination_column]->line = ent->line;
                (*board_map)[destination_line][destination_column]->column = ent->column;

                (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

                updates[2] = (*board_map)[ent->line][ent->column]->type;
                updates[3] = (*board_map)[ent->line][ent->column]->idx;

                //actualizar e meter a estrutura do bicho no destino
                ent->line = destination_line;
                ent->column = destination_column;

                (*board_map)[destination_line][destination_column] = ent;

                updates[0] = ent->type;
                updates[1] = ent->idx;

                updates[4] = -2;    //vazio
            }
            else{
                /* papa */
                if(ent->type == 2){ //pacman come fruta

                    //passar a estrutura da fruta para uma estrutura de free space e mudar as coordenadas
                    (*board_map)[destination_line][destination_column]->line = ent->line;
                    (*board_map)[destination_line][destination_column]->column = ent->column;
                    (*board_map)[destination_line][destination_column]->type = -1;

                    aux = (*board_map)[destination_line][destination_column]->idx;
                    (*board_map)[destination_line][destination_column]->idx = *free_space_counter;
                    (*free_space_list)[*free_space_counter] = (*board_map)[destination_line][destination_column];
                    (*free_space_counter)++;

                    (*fruits)[aux] = fruits[(*fruit_counter)-1];
                    (*fruits)[aux]->idx = aux;
                    (*fruits)[(*fruit_counter)-1] = NULL;
                    (*fruit_counter)--;

                    //----------- tirar do array de frutas e meter no array de free_spaces

                    (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

                    updates[2] = (*board_map)[ent->line][ent->column]->type;
                    updates[3] = (*board_map)[ent->line][ent->column]->idx;

                    //actualizar e meter a estrutura do bicho no destino
                    ent->line = destination_line;
                    ent->column = destination_column;

                    (*board_map)[destination_line][destination_column] = ent;

                    (ent->u_details->score)++;

                    ent->type = 5;

                    updates[0] = ent->type;
                    updates[1] = ent->idx;

                    updates[4] = -2;    //vazio

                }
                else if(ent->type == 3){    //monstro come pacman ou fruta

                    if ((*board_map)[destination_line][destination_column]->type == 0 || (*board_map)[destination_line][destination_column]->type == 1 ) //monstro come fruta
                    {
                        //passar a estrutura da fruta para uma estrutura de free space e mudar as coordenadas
                        (*board_map)[destination_line][destination_column]->line = ent->line;
                        (*board_map)[destination_line][destination_column]->column = ent->column;
                        (*board_map)[destination_line][destination_column]->type = -1;

                        aux = (*board_map)[destination_line][destination_column]->idx;
                        (*board_map)[destination_line][destination_column]->idx = *free_space_counter;
                        (*free_space_list)[*free_space_counter] = (*board_map)[destination_line][destination_column];
                        (*free_space_counter)++;

                        (*fruits)[aux] = fruits[(*fruit_counter)-1];
                        (*fruits)[aux]->idx = aux;
                        (*fruits)[(*fruit_counter)-1] = NULL;
                        (*fruit_counter)--;

                        //----------- tirar do array de frutas e meter no array de free_spaces

                        (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

                        updates[2] = (*board_map)[ent->line][ent->column]->type;
                        updates[3] = (*board_map)[ent->line][ent->column]->idx;

                        //actualizar e meter a estrutura do bicho no destino
                        ent->line = destination_line;
                        ent->column = destination_column;

                        (*board_map)[destination_line][destination_column] = ent;

                        (ent->u_details->score)++;

                        updates[0] = ent->type;
                        updates[1] = ent->idx;

                        updates[4] = -2;    //vazio
                    }
                    else if((*board_map)[destination_line][destination_column]->type == 2)  //monstro come pacman
                    {
                        random = (rand() % (*free_space_counter));

                        //actualiza-se as coordenadas do pacman e o ponteiro  para o random free_space passa a apontar para o pacman
                        (*board_map)[destination_line][destination_column]->column = (*free_space_list)[random]->column;
                        (*board_map)[destination_line][destination_column]->line = (*free_space_list)[random]->line;
                        (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*board_map)[destination_line][destination_column];
                        
                        updates[2] = 2;
                        updates[3] = (*board_map)[destination_line][destination_column]->idx;

                        //actualiza-se as coordenadas do free space para onde está o monstro
                        (*free_space_list)[random]->line = ent->line;
                        (*free_space_list)[random]->column = ent->column;

                        //actualiza-se as coordenadas e score do monstro e o ponteiro que apontava para o pacman, passa a apontar para o monstro
                        ent->line = destination_line;
                        ent->column = destination_column;
                        (ent->u_details->score)++;
                        (*board_map)[destination_line][destination_column] = ent;

                        updates[0] = 3;
                        updates[1] = ent->idx;

                        //o ponteiro da board que apontava para o monstro, passa a apontar para o free space
                        (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*free_space_list)[random];
                        
                        updates[4] = -1;
                        updates[5] = random;
                    }
                }
                else if(ent->type == 4 || ent->type == 5){  //super charged pacman come monstro ou fruta

                    if((*board_map)[destination_line][destination_column]->type == 0 || (*board_map)[destination_line][destination_column]->type == 1)  //pacman come fruta
                    {
                        //passar a estrutura da fruta para uma estrutura de free space e mudar as coordenadas
                        (*board_map)[destination_line][destination_column]->line = ent->line;
                        (*board_map)[destination_line][destination_column]->column = ent->column;
                        (*board_map)[destination_line][destination_column]->type = -1;

                        aux = (*board_map)[destination_line][destination_column]->idx;
                        (*board_map)[destination_line][destination_column]->idx = *free_space_counter;
                        (*free_space_list)[*free_space_counter] = (*board_map)[destination_line][destination_column];
                        (*free_space_counter)++;

                        (*fruits)[aux] = fruits[(*fruit_counter)-1];
                        (*fruits)[aux]->idx = aux;
                        (*fruits)[(*fruit_counter)-1] = NULL;
                        (*fruit_counter)--;

                        //----------- tirar do array de frutas e meter no array de free_spaces

                        (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

                        updates[2] = (*board_map)[ent->line][ent->column]->type;
                        updates[3] = (*board_map)[ent->line][ent->column]->idx;

                        //actualizar e meter a estrutura do bicho no destino
                        ent->line = destination_line;
                        ent->column = destination_column;

                        (*board_map)[destination_line][destination_column] = ent;

                        (ent->u_details->score)++;

                        ent->type = 5;

                        updates[0] = ent->type;
                        updates[1] = ent->idx;

                        updates[4] = -2;
                    }
                    else if((*board_map)[destination_line][destination_column]->type == 3){  //pacman come monstro

                        random = (rand() % (*free_space_counter));

                        //actualiza-se as coordenadas do monstro e o ponteiro na board para o random free_space passa a apontar para o monstro
                        (*board_map)[destination_line][destination_column]->column = (*free_space_list)[random]->column;
                        (*board_map)[destination_line][destination_column]->line = (*free_space_list)[random]->line;
                        (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*board_map)[destination_line][destination_column];
                        
                        updates[2] = 2;
                        updates[3] = (*board_map)[destination_line][destination_column]->idx;

                        //actualiza-se as coordenadas do free space para onde está o pacman
                        (*free_space_list)[random]->line = ent->line;
                        (*free_space_list)[random]->column = ent->column;

                        //actualiza-se as coordenadas e score do pacman e o ponteiro que apontava para o monstro, passa a apontar para o pacman
                        ent->line = destination_line;
                        ent->column = destination_column;
                        (ent->u_details->score)++;
                        (*board_map)[destination_line][destination_column] = ent;

                        //diminuir o type do charged pacman e, caso tenha comido 2 monstros desde a ultima fruta, passa para type 2 (pacman normal)
                        (ent->type)--;
                        if(ent->type == 3) ent->type = 2;

                        updates[0] = 3;
                        updates[1] = ent->idx;

                        //o ponteiro da board que apontava para o monstro, passa a apontar para o free space
                        (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*free_space_list)[random];
                        
                        updates[4] = -1;
                        updates[5] = random;

                    }
                }
            }
        }
        else if (ent->type < (*board_map)[destination_line][destination_column]->type)
        {
            //verificar se é do mesmo user -> troca
            if(ent->idx == (*board_map)[destination_line][destination_column]->idx){

                //actualizar e meter a estrutura de monstro/pacman do mesmo user no sitio onde estava antes o bicho
                (*board_map)[destination_line][destination_column]->line = ent->line;
                (*board_map)[destination_line][destination_column]->column = ent->column;

                (*board_map)[ent->line][ent->column] = (*board_map)[destination_line][destination_column];

                updates[2] = (*board_map)[ent->line][ent->column]->type;
                updates[3] = (*board_map)[ent->line][ent->column]->idx;

                //actualizar e meter a estrutura do bicho no destino
                ent->line = destination_line;
                ent->column = destination_column;

                (*board_map)[destination_line][destination_column] = ent;

                updates[0] = ent->type;
                updates[1] = ent->idx;

                updates[4] = -2;
            }
            else{
                /* é papado */
                
                if(ent->type == 2 && (*board_map)[destination_line][destination_column]->type == 3){    //pacman para cima de monstro

                    random = (rand() % (*free_space_counter));

                    //troca-se as coordenadas do pacman e da estrutura do free_space
                    aux = ent->column;
                    ent->column = (*free_space_list)[random]->column;
                    (*free_space_list)[random]->column = aux;

                    aux = ent->line;
                    ent->line = (*free_space_list)[random]->line;
                    (*free_space_list)[random]->line = aux;

                    (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*free_space_list)[random];
                    (*board_map)[ ent->line ][ ent->column ] = ent;

                    ((*board_map)[destination_line][destination_column]->u_details->score)++;

                    updates[0] = 2;
                    updates[1] = ent->idx;
                    updates[2] = -1;
                    updates[3] = (*free_space_list)[random]->idx;
                    updates[4] = -2;
                }
                //monstro para cima de charged pacman
                else if (ent->type == 3 && ((*board_map)[destination_line][destination_column]->type == 4 || (*board_map)[destination_line][destination_column]->type == 5))
                {
                    random = (rand() % (*free_space_counter));

                    //troca-se as coordenadas do pacman e da estrutura do free_space
                    aux = ent->column;
                    ent->column = (*free_space_list)[random]->column;
                    (*free_space_list)[random]->column = aux;

                    aux = ent->line;
                    ent->line = (*free_space_list)[random]->line;
                    (*free_space_list)[random]->line = aux;

                    (*board_map)[ (*free_space_list)[random]->line ][ (*free_space_list)[random]->column ] = (*free_space_list)[random];
                    (*board_map)[ ent->line ][ ent->column ] = ent;

                    ((*board_map)[destination_line][destination_column]->u_details->score)++;

                    updates[0] = 2;
                    updates[1] = ent->idx;
                    updates[2] = -1;
                    updates[3] = (*free_space_list)[random]->idx;

                    //diminuir o type do charged pacman e, caso tenha comido 2 monstros desde a ultima fruta, passa para type 2 (pacman normal)
                    ((*board_map)[destination_line][destination_column]->type)--;
                    if((*board_map)[destination_line][destination_column]->type == 3){
                        (*board_map)[destination_line][destination_column]->type = 2;
                        //caso o pacman deixe de estar charged, é preciso actualizar o boneco
                        updates[4] = 2;
                        updates[5] = (*board_map)[destination_line][destination_column]->idx;
                    }
                    else{
                        updates[4] = -2;
                    }
                } 
            }
        }
    }
}