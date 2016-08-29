#include <cstdio>
#include <cstdint>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

class Player {
};

SDL_Texture *textToTexture(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color = {0, 0, 0}) {
  SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface); 

  return texture;
}

SDL_Texture *loadTexture(SDL_Renderer *r, const char *path) {
  auto load = IMG_Load(path);

  if (load == nullptr) {
    printf("Failed to load image '%s'.\nIMG_Error: %s\n", path, IMG_GetError());
    return nullptr;
  }
  
  auto texture = SDL_CreateTextureFromSurface(r, load);

  if (texture == nullptr) {
    printf("Failed to create texture from '%s'.\nIMG_Error: %s\n", path, SDL_GetError());
    return nullptr;
  }

  SDL_FreeSurface(load);

  return texture;
}

class Map {
  private:
    uint8_t *map;

    uint32_t width;
    uint32_t height;
    uint32_t tileSize;

  public:
    Map(uint32_t w, uint32_t h, uint32_t tileSize = 16)
      : width(w)
      , height(h)
      , tileSize(tileSize)
      , map { new uint8_t [w * h] }
    {
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          get(x, y) = (y <= 0 || y >= height - 1 || x <= 0 || x >= width - 1);
        }
      }
    }

    ~Map() {
      delete [] map;
    }

    uint8_t & get(uint32_t x, uint32_t y) {
      return map[y * width + x];
    }

    void render(uint32_t ox, uint32_t oy, SDL_Renderer *r, TTF_Font *f) {
      auto wall  = loadTexture(r, "res/wall.png");
      auto floor = loadTexture(r, "res/floor.png");

      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          SDL_Rect rect {
            static_cast<int>(ox + x * tileSize),
            static_cast<int>(oy + y * tileSize),
            static_cast<int>(tileSize),
            static_cast<int>(tileSize),
          };

          if (get(x, y)) {
            SDL_RenderCopy(r, wall, nullptr, &rect);
          } else {
            SDL_RenderCopy(r, floor, nullptr, &rect);
          }
        }
      }

      SDL_DestroyTexture(wall);
      SDL_DestroyTexture(floor);
    }
};

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

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    printf("Failed to initialize SDL_image.\nIMG_Error: %s\n", IMG_GetError());
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

  /* Data */
  Map m(20, 20);

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

    m.render(0, 0, renderer, font);

    SDL_RenderPresent(renderer);
  }

  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}
