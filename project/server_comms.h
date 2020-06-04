#include "comms.h"

void send_NewClient(int n_clients, entity** pacmans, entity** monsters);

void send_Disconnect(int client_id, int n_clients, entity** pacmans);

void send_AllClients(int n_clients, entity** pacmans, entity** monsters);

void send_AllBricks(int client_socket, entity** bricks, int n_bricks);

void send_AllFruits(int client_socket, entity** fruits, int n_fruits);

void send_Move(int* updated,
               entity** pacmans,
               entity** monsters,
               int n_clients);

void send_Fruit(entity** fruits, int n_fruits, entity** pacmans, int n_clients);
