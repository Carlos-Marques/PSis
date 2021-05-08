#include <SDL2/SDL.h>
#include <pthread.h>
#include <unistd.h>
// gcc teste.c UI_library.c -o teste-UI -lpthread -lSDL2 -lSDL2_image

#include "UI_library.h"

// this variable will contain the identifier for our own event type
Uint32 Event_ShowImage;
// this data will be sent with the event
typedef struct Event_ShowImage_Data {
  int x, y;
  int pic;  // 0 cherry 1 lemon
} Event_ShowImage_Data;

void* thread_fruit(void* arg) {
  int x, y;
  SDL_Event new_event;
  Event_ShowImage_Data* event_data;
  int fruit = (int)arg;
  while (1) {
    // define the position of the next lemon
    x = random() % 50;
    y = random() % 20;

    // create the data that will contain the new lemon position
    event_data = malloc(sizeof(Event_ShowImage_Data));
    event_data->x = x;
    event_data->y = y;
    event_data->pic = fruit;

    // clear the event data
    SDL_zero(new_event);
    // define event type
    new_event.type = Event_ShowImage;
    // assign the event data
    new_event.user.data1 = event_data;
    // send the event
    SDL_PushEvent(&new_event);
    usleep(20000);
  }
}

int main() {
  SDL_Event event;
  int done = 0;

  // creates a windows and a board with 50x20 cases
  create_board_window(50, 20);

  Event_ShowImage = SDL_RegisterEvents(1);

  // monster and packman position
  int x = 0;
  int y = 0;
  // variable that defines what color to paint the monsters
  int click = 0;

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, thread_fruit, (void*)0);
  pthread_create(&thread_id, NULL, thread_fruit, (void*)1);

  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = SDL_TRUE;
      }
      // if the event is of type Event_ShowLemon
      if (event.type == Event_ShowImage) {
        // we get the data (created with the malloc)
        Event_ShowImage_Data* data = event.user.data1;
        // retrieve the x and y
        int x = data->x;
        int y = data->y;
        if (data->pic == 0) {
          // paint a cherry
          paint_cherry(data->x, data->y);
        } else {
          // we paint a lemon
          paint_lemon(data->x, data->y);
        }
        free(data);
      }
      // if the event is of type mousebuttondown
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        int window_x, window_y;
        window_x = event.button.x;
        window_y = event.button.y;
        printf("clicked on pixel x-%d y-%d\n", window_x, window_y);
        get_board_place(window_x, window_y, &x, &y);
        printf("clicked on board x-%d y-%d\n", x, y);
        paint_monster(x, y, 200, 20, 100);
      }
      if (event.type == SDL_MOUSEMOTION) {
        int x_new, y_new;

        // this fucntion return the place cwher the mouse cursor is
        get_board_place(event.motion.x, event.motion.y, &x_new, &y_new);
        // if the mluse moved toi anothe place
        if ((x_new != x) || (y_new != y)) {
          // the old place is cleared
          x = x_new;
          y = y_new;
          clear_place(x, y);
        }
      }
    }
  }
  printf("fim\n");
  close_board_windows();
}
