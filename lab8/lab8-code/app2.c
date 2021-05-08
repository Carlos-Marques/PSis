#include <SDL2/SDL.h>

//gcc teste.c UI_library.c -o teste-UI -lSDL2 -lSDL2_image

#include "UI_library.h"

void paint_bricks_fruits(){
	paint_brick(10, 10);
	paint_brick(10, 11);
	paint_brick(10, 12);
	paint_brick(9, 12);
	paint_brick(11, 12);
	paint_brick(10, 13);
	paint_brick(9, 13);
	paint_brick(11, 13);

	paint_lemon(30, 15);
	paint_cherry(30, 16);
}

int main(){

	SDL_Event event;
	int done = 0;

	//creates a windows and a board with 50x20 cases
	create_board_window(50, 20);

	//monster and packman position
	int x = 0;
	int y = 0;
	//variable that defines what color to paint the monstes
	int click = 0;

	//paint bricks
	paint_bricks_fruits();

	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
					done = SDL_TRUE;
			}
			// when the user clicks the left mouse button
			//the variable changes value
			if(event.type == SDL_MOUSEBUTTONDOWN){
				click = ! click;
			}
			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				int x_new, y_new;

				//this fucntion return the place cwher the mouse cursor is
				get_board_place(event.motion .x, event.motion .y,
												&x_new, &y_new);
				//if the mluse moved toi anothe place
				if((x_new != x) || (y_new != y)){
					//the old place is cleared
					clear_place(x, y);
					x = x_new;
					y=y_new;
					//decide what color to paint the monster
					//and paint it
					if(click){
						paint_monster(x, y , 200, 100, 7);
					}else{
						paint_monster(x, y , 7, 200, 100);
					}
					printf("move x-%d y-%d\n", x,y);
				}
			}
			//whenever the user clicks the left  arrow
			//the pacman moves
			if(event.type == SDL_KEYDOWN){
					int x_new, y_new;

					if (event.key.keysym.sym == SDLK_LEFT ){
						//the olde place is cleared
						clear_place(x, y);

						//new position ios calculated
						x = random()%50;
						y=random()%20;
						//paint the pacman
						paint_pacman(x, y , 7, 100, 200);
						printf("move x-%d y-%d\n", x,y);
					}
				}
		}
	}
	
	printf("fim\n");
	close_board_windows();
}
