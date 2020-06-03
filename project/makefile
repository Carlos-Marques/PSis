client: client.c client_comms.c pacman_structs.c UI_library.c
	gcc client.c client_comms.c pacman_structs.c UI_library.c -o client -lSDL2 -lSDL2_image -pthread

server: server.c server_comms.c pacman_structs.c UI_library.c
	gcc server.c server_comms.c pacman_structs.c UI_library.c -o server -lSDL2 -lSDL2_image -pthread

pacman: pacman.c pacman_structs.c UI_library.c gameLogic.c
	gcc pacman.c pacman_structs.c UI_library.c gameLogic.c -o pacman -lSDL2 -lSDL2_image -pthread
