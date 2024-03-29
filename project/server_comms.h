#include "comms.h"

Uint32 Event_Move, Event_NewUser, Event_Disconnect, Event_Inactivity,
    Event_ScoreBoard;

void send_NewClient(int n_clients, entity** pacmans, entity** monsters);

void send_Disconnect(int client_id, int n_clients, entity** pacmans);

void send_AllClients(int n_clients, entity** pacmans, entity** monsters);

void send_AllBricks(entity* client, entity** bricks, int n_bricks);

void send_AllFruits(entity* client, entity** fruits, int n_fruits);

void send_Move(int* updated,
               entity** pacmans,
               entity** monsters,
               int n_clients);

void send_Fruit(entity** fruits, int n_fruits, entity** pacmans, int n_clients);

void send_ScoreBoard(entity** pacmans, int sendTo, int n_users);

void send_Clear(entity** pacmans, int n_clients, coords* clear_space);
