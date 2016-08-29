#include <cstdio>

#include <SDL.h>
#include <SDL_ttf.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

int main() {
  /* All systems go! */
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("Failed to initilize SDL.\nSDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  if (TTF_Init() < 0) {
    printf("Failed to initialize SDL_ttf.\nTTF_Error: %s\n", TTF_GetError());
    return 1;
  }

  /* Create the window */
  SDL_Window *window = SDL_CreateWindow("Rogue",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  if (window == nullptr) {
    printf("Failed to create window.\nSDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  /* Create the renderer */
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == nullptr) {
    printf("Failed to create rederer.\nSDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

  /* Set up text rendering */
  TTF_Font *font = TTF_OpenFont("res/PxPlus_IBM_VGA8.ttf", 28);

  if (font == nullptr) {
    printf("Failed to load font.\nTTF_Error: %s\n", TTF_GetError());
    return 1;
  }

  SDL_Surface *_text = TTF_RenderText_Solid(font, "Hello, world!", {0, 0, 0});
  SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, _text);
  SDL_FreeSurface(_text); 

  SDL_Rect text_rect { 100, 100, 0, 0 };

  SDL_QueryTexture(text, nullptr, nullptr, &text_rect.w, &text_rect.h);

  /* Main loop */
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, text, nullptr, &text_rect);
    SDL_RenderPresent(renderer);
  }

  TTF_CloseFont(font);
  SDL_DestroyTexture(text);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}
