#include <SDL2/SDL.h>
#include <pthread.h>
#include <unistd.h>
//gcc teste.c UI_library.c -o teste-UI -lpthread -lSDL2 -lSDL2_image

#include "UI_library.h"


// this variable will contain the identifier for our own event type
Uint32 Event_ShowLemon;
//this data will be sent with the event
typedef struct Event_ShowLemon_Data{
	int x, y;
} Event_ShowLemon_Data;


void * thread_lemon(void * arg){
	int x, y;
	SDL_Event new_event;
	Event_ShowLemon_Data * event_data;

	while(1){

		//define the position of the next lemon
		x = random()%50;
		y = random()%20;

		//create the data that will contain the new lemon position
		event_data = malloc(sizeof(Event_ShowLemon_Data));
		event_data->x = x;
		event_data->y = y;

		// clear the event data
		SDL_zero(new_event); 
		// define event type
		new_event.type = Event_ShowLemon;
		//assign the event data
		new_event.user.data1 = event_data;
		// send the event
		SDL_PushEvent(&new_event);
		usleep(250000);
	}
}

int main(){

	SDL_Event event;
	int done = 0;

	//creates a windows and a board with 50x20 cases
	create_board_window(50, 20);

	Event_ShowLemon =  SDL_RegisterEvents(1);

	//monster and packman position
	int x = 0;
	int y = 0;
	//variable that defines what color to paint the monsters
	int click = 0;

	pthread_t thread_id;
	pthread_create(&thread_id, NULL,thread_lemon, NULL);



	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT){
				done = SDL_TRUE;
			}
			// if the event is of type Event_ShowLemon
			if(event.type == Event_ShowLemon){
				// we get the data (created with the malloc)
				Event_ShowLemon_Data * data = event.user.data1;
				// retrieve the x and y
				int x = data->x;
				int y = data->y;
				// we paint a lemon
				paint_lemon(data->x, data->y);
				free(data);
			}
			//if the event is of type mousebuttondown
			if(event.type == SDL_MOUSEBUTTONDOWN){
				int board_x, board_y;
				int window_x, window_y;
				window_x = event.button.x;
				window_y = event.button.y;
				printf("clicked on pixel x-%d y-%d\n", window_x, window_y);
				get_board_place(window_x, window_y, & board_x, &board_y);
				printf("clicked on board x-%d y-%d\n", board_x, board_y);
				clear_place(board_x, board_y);
			}
		}
	}
	printf("fim\n");
	close_board_windows();
}
