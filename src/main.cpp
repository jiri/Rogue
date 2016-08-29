#include <cstdio>

#include <SDL.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

int main() {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("Failed to initilize SDL.\nSDL_Error: %s\n", SDL_GetError());
  }

  SDL_Window *window = SDL_CreateWindow("Rogue",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  if (window == nullptr) {
    printf("Failed to create window.\nSDL_Error: %s\n", SDL_GetError());
  }

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  bool quit = false;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    SDL_FillRect(surface, nullptr,
        SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));

    SDL_UpdateWindowSurface(window);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
}
