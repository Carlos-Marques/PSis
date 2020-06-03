#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "UI_library.h"
#include "gameLogic.h"

void * clientThread(void * arg);
void * serverThread(void * argc);