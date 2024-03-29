#include "gameLogic.h"

void* scoreboardTimerThread();

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
                    int* n_fruits,
                    int n_lines,
                    int n_cols,
                    int* fruit_cap,
                    int max_clients);

void handle_Disconnect(entity** pacmans,
                       entity** monsters,
                       entity** free_spaces,
                       entity** fruits,
                       int n_lines,
                       int n_cols,
                       int client_id,
                       int* n_free_spaces,
                       int* n_clients,
                       int* n_fruits,
                       int* fruit_cap);

void handle_Inactivity(entity* character,
                       entity** free_spaces,
                       entity** pacmans,
                       entity** monsters,
                       entity*** board,
                       int n_free_spaces,
                       int n_clients);

void handle_ScoreBoard(int n_users, entity** pacmans);
