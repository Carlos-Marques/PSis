#include "gameLogic.h"

void* fruitRespawn_Thread() {
  pthread_t thread_id;
  SDL_Event new_event;

  sleep(2);

  SDL_zero(new_event);
  new_event.type = Event_RespawnFruit;
  SDL_PushEvent(&new_event);
  return (NULL);
}

void handle_mov_init(event_struct* move_data,
                          entity*** board,
                          int n_lines,
                          int n_cols,
                          entity** pacmans,
                          entity** monsters,
                          entity** fruits,
                          entity** free_spaces,
                          int* fruit_counter,
                          int* free_space_counter,
                          int n_clients){

  int* updates = (int*)malloc(sizeof(int) * 6);
  int to_send[2];

  int n_fruits = (*fruit_counter), n_free_spaces = (*free_space_counter);

  if (move_data->type)  // pacman
  {
    handle_mov(pacmans[move_data->client->idx]->type,
                move_data->client->idx, move_data->dir, board, n_lines,
                n_cols, pacmans, monsters, fruits, free_spaces,
                &n_fruits, &n_free_spaces, updates);
  } else  // monster
  {
    handle_mov(monsters[move_data->client->idx]->type,
                move_data->client->idx, move_data->dir, board, n_lines,
                n_cols, pacmans, monsters, fruits, free_spaces,
                &n_fruits, &n_free_spaces, updates);
  }
  
  for (int a = 0; a < 6 && updates[a] != -2; a += 2) {
    if (updates[a] == 2)  // Pacman
    {
      printf("pacman\n");
      paint_pacman(pacmans[updates[a + 1]]->column,
                    pacmans[updates[a + 1]]->line,
                    pacmans[updates[a + 1]]->u_details->r,
                    pacmans[updates[a + 1]]->u_details->g,
                    pacmans[updates[a + 1]]->u_details->b);

      to_send[0] = 1;
      to_send[1] = updates[a + 1];

      send_Move(to_send, pacmans, monsters, n_clients);
      printf("pacman sent\n");

    } else if (updates[a] == 3)  // Monster
    {
      printf("monster\n");
      paint_monster(monsters[updates[a + 1]]->column,
                    monsters[updates[a + 1]]->line,
                    monsters[updates[a + 1]]->u_details->r,
                    monsters[updates[a + 1]]->u_details->g,
                    monsters[updates[a + 1]]->u_details->b);

      to_send[0] = 0;
      to_send[1] = updates[a + 1];
      send_Move(to_send, pacmans, monsters, n_clients);

      printf("monster sent\n");
    } else if (updates[a] == -1)  // space
    {
      clear_place(free_spaces[updates[a + 1]]->column,
                  free_spaces[updates[a + 1]]->line);

    } else if (updates[a] == 4 || updates[a] == 5)  // Charged_Pacman
    {
      paint_powerpacman(pacmans[updates[a + 1]]->column,
                        pacmans[updates[a + 1]]->line, 255, 0, 0);

      to_send[0] = 2;
      to_send[1] = updates[a + 1];
      send_Move(to_send, pacmans, monsters, n_clients);
    }
  }

  (*fruit_counter) = n_fruits;
  (*free_space_counter) = n_free_spaces;

  free(updates);
}

void handle_mov(int type,
                int idx,
                int dir,
                entity*** board_map,
                int n_lin,
                int n_col,
                entity** pacmans,
                entity** monsters,
                entity** fruits,
                entity** free_space_list,
                int* fruit_counter,
                int* free_space_counter,
                int* updates) {
  int random, destination_line, destination_column, aux, bounce = 0;
  entity *ent, aux_ent;

  if (type == 2 || type == 4 || type == 5) {  // pacman

    ent = (pacmans)[idx];
  } else if (type == 3) {  // monster

    ent = monsters[idx];
  }

  get_destination(dir, n_lin, n_col, &destination_line, &destination_column,
                  board_map, ent);

  if (destination_line == ent->line &&
      destination_column == ent->column) {  // double bounce, no updates in SDL
    updates[0] = -2;                        // vazio
  } else {
    if (board_map[destination_line][destination_column]->type ==
        -1)  // entity mooove to free space
    {
      move_to_freeSpace(destination_line, destination_column, board_map, ent,
                        updates);
    }
    // same types
    else if (ent->type ==
                 board_map[destination_line][destination_column]->type ||
             ((ent->type == 2 || ent->type == 4 || ent->type == 5) &&
              (board_map[destination_line][destination_column]->type == 2 ||
               board_map[destination_line][destination_column]->type == 4 ||
               board_map[destination_line][destination_column]->type ==
                   5)))  // second condition for various types of pacman
    {
      switch_places(destination_line, destination_column, board_map, ent,
                    updates);
    } else if (ent->type >
               board_map[destination_line][destination_column]->type) {
      // verificar se é do mesmo user -> troca
      if ((ent->idx == board_map[destination_line][destination_column]->idx) &&
          (board_map[destination_line][destination_column]->type != 0) &&
          (board_map[destination_line][destination_column]->type != 1)) {
        switch_places(destination_line, destination_column, board_map, ent,
                      updates);
      } else {
        /* papa */
        if (ent->type == 2) {  // pacman come fruta

          pacman_eats_fruit(destination_line, destination_column, board_map,
                            ent, updates, &free_space_counter, free_space_list,
                            &fruit_counter, fruits);
        } else if (ent->type == 3) {  // monstro come pacman ou fruta

          if (board_map[destination_line][destination_column]->type == 0 ||
              board_map[destination_line][destination_column]->type ==
                  1)  // monstro come fruta
          {
            monster_eats_fruit(destination_line, destination_column, board_map,
                               ent, updates, &free_space_counter,
                               free_space_list, &fruit_counter, fruits);
          } else if (board_map[destination_line][destination_column]->type ==
                     2)  // monstro come pacman
          {
            monster_eats_pacman(destination_line, destination_column, board_map,
                                ent, updates, *free_space_counter,
                                free_space_list);
          }
        } else if (ent->type == 4 ||
                   ent->type ==
                       5) {  // super charged pacman come monstro ou fruta

          if (board_map[destination_line][destination_column]->type == 0 ||
              board_map[destination_line][destination_column]->type ==
                  1)  // pacman come fruta
          {
            pacman_eats_fruit(destination_line, destination_column, board_map,
                              ent, updates, &free_space_counter,
                              free_space_list, &fruit_counter, fruits);
          } else if (board_map[destination_line][destination_column]->type ==
                     3)  // pacman come monstro
          {
            superPacman_eats_monster(destination_line, destination_column,
                                     board_map, ent, updates,
                                     *free_space_counter, free_space_list);
          }
        }
      }
    } else if (ent->type <
               board_map[destination_line][destination_column]->type) {
      // verificar se é do mesmo user -> troca
      if (ent->idx == board_map[destination_line][destination_column]->idx) {
        switch_places(destination_line, destination_column, board_map, ent,
                      updates);
      } else {
        /* é papado */

        if (ent->type == 2 &&
            board_map[destination_line][destination_column]->type ==
                3) {  // pacman para cima de monstro

          pacman_into_monster(destination_line, destination_column, board_map,
                              ent, updates, *free_space_counter,
                              free_space_list);
        }
        // monstro para cima de charged pacman
        else if (ent->type == 3 &&
                 (board_map[destination_line][destination_column]->type == 4 ||
                  board_map[destination_line][destination_column]->type == 5)) {
          monster_into_superPacman(destination_line, destination_column,
                                   board_map, ent, updates, *free_space_counter,
                                   free_space_list);
        }
      }
    }
  }
}

void get_destination(int dir,
                     int n_lin,
                     int n_col,
                     int* destination_line,
                     int* destination_column,
                     entity*** board_map,
                     entity* ent) {
  if (dir == 0) {  // down

    *destination_line = ent->line + 1;
    *destination_column = ent->column;

    if (*destination_line >= n_lin) {  // out of window - bounce

      if (*destination_line - 2 < 0) {  // bounced out of window - double bounce
        *destination_line -= 1;
      } else if (board_map[*destination_line - 2][*destination_column]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_line -= 1;
      } else {
        *destination_line -= 2;  // bounce
      }
    } else if (board_map[*destination_line][*destination_column]->type ==
               6) {  // into wall - bounce

      if (*destination_line - 2 < 0) {  // bounced out of window - double bounce
        *destination_line -= 1;
      } else if (board_map[*destination_line - 2][*destination_column]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_line -= 1;
      } else {
        *destination_line -= 2;  // bounce
      }
    }
  } else if (dir == 1) {  // up

    *destination_line = ent->line - 1;
    *destination_column = ent->column;

    if (*destination_line < 0) {  // out of window - bounce

      if (*destination_line + 2 >=
          n_lin) {  // bounced out of window - double bounce
        *destination_line += 1;
      } else if (board_map[*destination_line + 2][*destination_column]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_line += 1;
      } else {
        *destination_line += 2;  // bounce
      }
    } else if (board_map[*destination_line][*destination_column]->type ==
               6) {  // into wall - bounce

      if (*destination_line + 2 >=
          n_lin) {  // bounced out of window - double bounce
        *destination_line += 1;
      } else if (board_map[*destination_line + 2][*destination_column]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_line += 1;
      } else {
        *destination_line += 2;
      }
    }
  } else if (dir == 2) {  // left

    *destination_line = ent->line;
    *destination_column = ent->column - 1;

    if (*destination_column < 0) {  // out of window - bounce

      if (*destination_column + 2 >=
          n_col) {  // bounced out of window - double bounce
        *destination_column += 1;
      } else if (board_map[*destination_line][*destination_column + 2]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_column += 1;
      } else {
        *destination_column += 2;  // bounce
      }
    } else if (board_map[*destination_line][*destination_column]->type ==
               6) {  // into wall - bounce

      if (*destination_column + 2 >=
          n_col) {  // bounced out of window - double bounce
        *destination_column += 1;
      } else if (board_map[*destination_line][*destination_column + 2]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_column += 1;
      } else {
        *destination_column += 2;
      }
    }
  } else if (dir == 3) {  // right

    *destination_line = ent->line;
    *destination_column = ent->column + 1;

    if (*destination_column >= n_col) {  // out of window - bounce

      if (*destination_column - 2 <
          0) {  // bounced out of window - double bounce
        *destination_column -= 1;
      } else if (board_map[*destination_line][*destination_column - 2]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_column -= 1;
      } else {
        *destination_column -= 2;  // bounce
      }
    } else if (board_map[*destination_line][*destination_column]->type ==
               6) {  // into wall - bounce

      if (*destination_column - 2 <
          0) {  // bounced out of window - double bounce
        *destination_column -= 1;
      } else if (board_map[*destination_line][*destination_column - 2]->type ==
                 6) {  // bounced into wall - double bounce
        *destination_column -= 1;
      } else {
        *destination_column -= 2;
      }
    }
  }
}

void move_to_freeSpace(int destination_line,
                       int destination_column,
                       entity*** board_map,
                       entity* ent,
                       int* updates) {
  // actualizar e meter a estrutura de free space no sitio onde estava antes o
  // bicho

  board_map[destination_line][destination_column]->line = ent->line;
  board_map[destination_line][destination_column]->column = ent->column;

  board_map[ent->line][ent->column] =
      board_map[destination_line][destination_column];

  updates[2] = board_map[ent->line][ent->column]->type;
  updates[3] = board_map[ent->line][ent->column]->idx;

  // actualizar e meter a estrutura do bicho no destino
  ent->line = destination_line;
  ent->column = destination_column;

  board_map[destination_line][destination_column] = ent;

  updates[0] = ent->type;
  updates[1] = ent->idx;

  updates[4] = -2;  // vazio
}

void switch_places(int destination_line,
                   int destination_column,
                   entity*** board_map,
                   entity* ent,
                   int* updates) {
  printf("\n\n----------TROCA---------\n\n");
  /* troca */
  // actualizar e meter a estrutura de free space no sitio onde estava antes o
  // bicho
  board_map[destination_line][destination_column]->line = ent->line;
  board_map[destination_line][destination_column]->column = ent->column;

  board_map[ent->line][ent->column] =
      board_map[destination_line][destination_column];

  updates[2] = board_map[ent->line][ent->column]->type;
  updates[3] = board_map[ent->line][ent->column]->idx;

  // actualizar e meter a estrutura do bicho no destino
  ent->line = destination_line;
  ent->column = destination_column;

  board_map[destination_line][destination_column] = ent;

  updates[0] = ent->type;
  updates[1] = ent->idx;

  updates[4] = -2;
}
//---------------------------------------
void pacman_eats_fruit(int destination_line,
                       int destination_column,
                       entity*** board_map,
                       entity* ent,
                       int* updates,
                       int** free_space_counter,
                       entity** free_space_list,
                       int** fruit_counter,
                       entity** fruits) {
  int aux;
  pthread_t thread_id;

  // passar a estrutura da fruta para uma estrutura de free space e mudar as
  // coordenadas
  board_map[destination_line][destination_column]->line = ent->line;
  board_map[destination_line][destination_column]->column = ent->column;
  board_map[destination_line][destination_column]->type = -1;

  aux = board_map[destination_line][destination_column]->idx;
  board_map[destination_line][destination_column]->idx = **free_space_counter;
  free_space_list[**free_space_counter] =
      board_map[destination_line][destination_column];
  (**free_space_counter)++;

  if ((**fruit_counter) != 1) {
    fruits[aux] = fruits[(**fruit_counter) - 1];
    fruits[aux]->idx = aux;
  }
  fruits[(**fruit_counter) - 1] = NULL;
  (**fruit_counter)--;

  board_map[ent->line][ent->column] =
      board_map[destination_line][destination_column];

  updates[2] = board_map[ent->line][ent->column]->type;  // free space
  updates[3] = board_map[ent->line][ent->column]->idx;

  // actualizar e meter a estrutura do bicho no destino
  ent->line = destination_line;
  ent->column = destination_column;

  board_map[destination_line][destination_column] = ent;

  (ent->u_details->score)++;

  ent->type = 5;

  updates[0] = ent->type;
  updates[1] = ent->idx;

  updates[4] = -2;  // vazio

  pthread_create(&thread_id, NULL, fruitRespawn_Thread, NULL);
}
//---------------------------------------
void monster_eats_fruit(int destination_line,
                        int destination_column,
                        entity*** board_map,
                        entity* ent,
                        int* updates,
                        int** free_space_counter,
                        entity** free_space_list,
                        int** fruit_counter,
                        entity** fruits) {
  int aux;
  pthread_t thread_id;

  // passar a estrutura da fruta para uma estrutura de free space e mudar as
  // coordenadas
  board_map[destination_line][destination_column]->line = ent->line;
  board_map[destination_line][destination_column]->column = ent->column;
  board_map[destination_line][destination_column]->type = -1;

  aux = board_map[destination_line][destination_column]->idx;
  board_map[destination_line][destination_column]->idx = **free_space_counter;
  free_space_list[**free_space_counter] =
      board_map[destination_line][destination_column];
  (**free_space_counter)++;

  if ((**fruit_counter) != 1) {
    fruits[aux] = fruits[(**fruit_counter) - 1];
    fruits[aux]->idx = aux;
  }
  fruits[(**fruit_counter) - 1] = NULL;
  (**fruit_counter)--;

  board_map[ent->line][ent->column] =
      board_map[destination_line][destination_column];

  updates[2] = board_map[ent->line][ent->column]->type;
  updates[3] = board_map[ent->line][ent->column]->idx;

  // actualizar e meter a estrutura do bicho no destino
  ent->line = destination_line;
  ent->column = destination_column;

  board_map[destination_line][destination_column] = ent;

  (ent->u_details->score)++;

  updates[0] = ent->type;
  updates[1] = ent->idx;

  updates[4] = -2;  // vazio

  pthread_create(&thread_id, NULL, fruitRespawn_Thread, NULL);
}

void monster_eats_pacman(int destination_line,
                         int destination_column,
                         entity*** board_map,
                         entity* ent,
                         int* updates,
                         int free_space_counter,
                         entity** free_space_list) {
  int random = (rand() % free_space_counter);

  // actualiza-se as coordenadas do pacman e o ponteiro  para o random
  // free_space passa a apontar para o pacman
  board_map[destination_line][destination_column]->column =
      free_space_list[random]->column;
  board_map[destination_line][destination_column]->line =
      free_space_list[random]->line;
  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      board_map[destination_line][destination_column];

  updates[2] = 2;
  updates[3] = board_map[destination_line][destination_column]->idx;

  // actualiza-se as coordenadas do free space para onde está o monstro
  free_space_list[random]->line = ent->line;
  free_space_list[random]->column = ent->column;

  // actualiza-se as coordenadas e score do monstro e o ponteiro que apontava
  // para o pacman, passa a apontar para o monstro
  ent->line = destination_line;
  ent->column = destination_column;
  (ent->u_details->score)++;
  board_map[destination_line][destination_column] = ent;

  updates[0] = 3;
  updates[1] = ent->idx;

  // o ponteiro da board que apontava para o monstro, passa a apontar para o
  // free space
  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      free_space_list[random];

  updates[4] = -1;
  updates[5] = random;
}

void superPacman_eats_monster(int destination_line,
                              int destination_column,
                              entity*** board_map,
                              entity* ent,
                              int* updates,
                              int free_space_counter,
                              entity** free_space_list) {
  int random = (rand() % free_space_counter);

  // actualiza-se as coordenadas do monstro e o ponteiro na board para o random
  // free_space passa a apontar para o monstro passa o monstro para o free space
  board_map[destination_line][destination_column]->column =
      free_space_list[random]->column;
  board_map[destination_line][destination_column]->line =
      free_space_list[random]->line;
  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      board_map[destination_line][destination_line];

  // envia os updates do monstro
  updates[2] = board_map[destination_line][destination_column]->type;
  updates[3] = board_map[destination_line][destination_column]->idx;

  // actualiza-se as coordenadas do free space para onde está o pacman
  free_space_list[random]->line = ent->line;
  free_space_list[random]->column = ent->column;

  // actualiza-se as coordenadas e score do pacman e o ponteiro que apontava
  // para o monstro, passa a apontar para o pacman
  ent->line = destination_line;
  ent->column = destination_column;
  (ent->u_details->score)++;
  board_map[destination_line][destination_column] = ent;

  // diminuir o type do charged pacman e, caso tenha comido 2 monstros desde a
  // ultima fruta, passa para type 2 (pacman normal)
  (ent->type)--;
  if (ent->type == 3)
    ent->type = 2;

  updates[0] = ent->type;
  updates[1] = ent->idx;

  // o ponteiro da board que apontava para o monstro, passa a apontar para o
  // free space
  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      free_space_list[random];

  updates[4] = -1;
  updates[5] = random;
}

void pacman_into_monster(int destination_line,
                         int destination_column,
                         entity*** board_map,
                         entity* ent,
                         int* updates,
                         int free_space_counter,
                         entity** free_space_list) {
  int random = (rand() % free_space_counter);

  // troca-se as coordenadas do pacman e da estrutura do free_space
  int aux = ent->column;
  ent->column = free_space_list[random]->column;
  free_space_list[random]->column = aux;

  aux = ent->line;
  ent->line = free_space_list[random]->line;
  free_space_list[random]->line = aux;

  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      free_space_list[random];
  board_map[ent->line][ent->column] = ent;

  (board_map[destination_line][destination_column]->u_details->score)++;

  updates[0] = 2;
  updates[1] = ent->idx;
  updates[2] = -1;
  updates[3] = free_space_list[random]->idx;
  updates[4] = -2;
}

void monster_into_superPacman(int destination_line,
                              int destination_column,
                              entity*** board_map,
                              entity* ent,
                              int* updates,
                              int free_space_counter,
                              entity** free_space_list) {
  int random = (rand() % free_space_counter);

  // troca-se as coordenadas do pacman e da estrutura do free_space
  int aux = ent->column;
  ent->column = free_space_list[random]->column;
  free_space_list[random]->column = aux;

  aux = ent->line;
  ent->line = free_space_list[random]->line;
  free_space_list[random]->line = aux;

  board_map[free_space_list[random]->line][free_space_list[random]->column] =
      free_space_list[random];
  board_map[ent->line][ent->column] = ent;

  (board_map[destination_line][destination_column]->u_details->score)++;

  updates[0] = ent->type;
  updates[1] = ent->idx;
  updates[2] = -1;
  updates[3] = free_space_list[random]->idx;

  // diminuir o type do charged pacman e, caso tenha comido 2 monstros desde a
  // ultima fruta, passa para type 2 (pacman normal)
  (board_map[destination_line][destination_column]->type)--;
  if (board_map[destination_line][destination_column]->type == 3) {
    board_map[destination_line][destination_column]->type = 2;
    // caso o pacman deixe de estar charged, é preciso actualizar o boneco
    updates[4] = board_map[destination_line][destination_column]->type;
    updates[5] = board_map[destination_line][destination_column]->idx;
  } else {
    updates[4] = -2;
  }
}

void respawn_fruit(int* free_space_counter,
                   entity** free_space_list,
                   int* fruit_counter,
                   entity** fruits) {

  int random = (rand() % (*free_space_counter));
  int new_fruit_type = rand() % 2;

  // trocar o tipo para um de fruta
  free_space_list[random]->type = new_fruit_type;
  // apontar para a estrutura da nova fruta no array das frutas
  fruits[(*fruit_counter) - 1] = free_space_list[random];
  // actualiza o index e o fruit counter
  fruits[(*fruit_counter) - 1]->idx = (*fruit_counter) - 1;

  if(new_fruit_type == 0){    //cherry
    paint_cherry(fruits[(*fruit_counter) - 1]->column , fruits[(*fruit_counter) - 1]->line);
    //TO DO: SEND CHERRY
  }
  else{                       //lemon
    paint_lemon(fruits[(*fruit_counter) - 1]->column , fruits[(*fruit_counter) - 1]->line);
    //TO DO: SEND LEMON
  }

  (*fruit_counter)++;

  // meter o espaço vazio dos free spaces a apontar para a ultima posição
  free_space_list[random] = free_space_list[(*free_space_counter) - 1];
  free_space_list[random]->idx = random;
  free_space_list[(*free_space_counter) - 1] = NULL;
  (*free_space_counter)--;
}
