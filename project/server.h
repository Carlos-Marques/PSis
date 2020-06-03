
#include "gameLogic.h"

void* inactivityThread(void* args);

void* clientThread(void* args);

void* connectionThread();

void handle_NewUser(user_details* new_client_details,
                    entity** pacmans,
                    entity** monsters,
                    entity** free_spaces,
                    entity** bricks,
                    entity** fruits,
                    int* n_clients,
                    int* n_free_spaces,
                    int n_bricks,
                    int n_fruits,
                    int n_lines,
                    int n_cols);

void handle_Disconnect(entity** pacmans,
                       entity** monsters,
                       entity** free_spaces,
                       int n_lines,
                       int n_cols,
                       int client_id,
                       int* n_free_spaces,
                       int* n_clients);

void handle_Inactivity(entity* character,
                       entity** free_spaces,
                       entity** pacmans,
                       entity** monsters,
                       entity*** board,
                       int n_free_spaces,
                       int n_clients);
