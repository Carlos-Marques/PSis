client: client.c client_comms.c pacman_structs.c UI_library.c
	gcc-8 client.c client_comms.c pacman_structs.c UI_library.c -o client -lSDL2 -lSDL2_image -pthread -Wall

server: server.c server_comms.c pacman_structs.c UI_library.c gameLogic.c
	gcc-8 server.c server_comms.c pacman_structs.c UI_library.c gameLogic.c -o server -lSDL2 -lSDL2_image -pthread -Wall
