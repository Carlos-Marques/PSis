#include "comms.h"

void rcv_NewClient(client_data** clients, int server_socket, int* n_users);

void rcv_Disconnect(client_data** clients, int server_socket, int* n_clients);

void rcv_AllClients(client_data** clients, int server_socket, int* n_clients);

void rcv_AllBricks(int server_socket);

void rcv_AllFruits(int server_socket);

void rcv_MovePacman(int server_socket,
                    client_data** clients,
                    int clean,
                    int super);

void rcv_MoveMonster(int server_socket, client_data** clients, int clean);

void rcv_Cherry(int server_socket);

void rcv_Lemon(int server_socket);

void rcv_ScoreBoard(int server_socket, int n_clients);

void rcv_Clear(int server_socket);
