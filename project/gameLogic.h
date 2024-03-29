#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "server_comms.h"

Uint32 Event_RespawnFruit;



void* fruitRespawn_Thread();

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
                     int n_clients,
                     activeFruitThread* head_activeFruitThread,
                     activeFruitThread* tail_activeFruitThread);

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
                int* updates,
                activeFruitThread* head_activeFruitThread,
                activeFruitThread* tail_activeFruitThread);

void get_destination(int dir,
                     int n_lin,
                     int n_col,
                     int* destination_line,
                     int* destination_column,
                     entity*** board_map,
                     entity* ent);

void move_to_freeSpace(int destination_line,
                       int destination_column,
                       entity*** board_map,
                       entity* ent,
                       int* updates);

void switch_places(int destination_line,
                   int destination_column,
                   entity*** board_map,
                   entity* ent,
                   int* updates);

void pacman_eats_fruit(int destination_line,
                       int destination_column,
                       entity*** board_map,
                       entity* ent,
                       int* updates,
                       int** free_space_counter,
                       entity** free_space_list,
                       int** fruit_counter,
                       entity** fruits, activeFruitThread* head_activeFruitThread, activeFruitThread* tail_activeFruitThread);

void monster_eats_fruit(int destination_line,
                        int destination_column,
                        entity*** board_map,
                        entity* ent,
                        int* updates,
                        int** free_space_counter,
                        entity** free_space_list,
                        int** fruit_counter,
                        entity** fruits, activeFruitThread* head_activeFruitThread, activeFruitThread* tail_activeFruitThread);

void monster_eats_pacman(int destination_line,
                         int destination_column,
                         entity*** board_map,
                         entity* ent,
                         int* updates,
                         int free_space_counter,
                         entity** free_space_list);

void superPacman_eats_monster(int destination_line,
                              int destination_column,
                              entity*** board_map,
                              entity* ent,
                              int* updates,
                              int free_space_counter,
                              entity** free_space_list);

void pacman_into_monster(int destination_line,
                         int destination_column,
                         entity*** board_map,
                         entity* ent,
                         int* updates,
                         int free_space_counter,
                         entity** free_space_list);

void monster_into_superPacman(int destination_line,
                              int destination_column,
                              entity*** board_map,
                              entity* ent,
                              int* updates,
                              int free_space_counter,
                              entity** free_space_list);

void respawn_fruit(int* free_space_counter,
                   entity** free_space_list,
                   int* fruit_counter,
                   entity** fruits,
                   int fruit_cap);
