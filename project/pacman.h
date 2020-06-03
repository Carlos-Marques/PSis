#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "UI_library.h"
#include "gameLogic.h"

void* clientThread(void* arg);
void* serverThread(void* argc);
