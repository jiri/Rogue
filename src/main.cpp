#include <cstdio>
#include <cstdint>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

SDL_Texture *textToTexture(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color = {0, 0, 0}) {
  SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface); 

  return texture;
}

SDL_Texture *loadTexture(SDL_Renderer *r, std::string path) {
  auto load = IMG_Load(path.c_str());

  if (load == nullptr) {
    printf("Failed to load image '%s'.\nIMG_Error: %s\n", path.c_str(), IMG_GetError());
    return nullptr;
  }
  
  auto texture = SDL_CreateTextureFromSurface(r, load);

  if (texture == nullptr) {
    printf("Failed to create texture from '%s'.\nIMG_Error: %s\n", path.c_str(), SDL_GetError());
    return nullptr;
  }

  SDL_FreeSurface(load);

  return texture;
}

class Tile {
  private:
    SDL_Texture *texture;

  public:
    Tile(SDL_Renderer *r, std::string name) {
      texture = loadTexture(r, "res/" + name + ".png");
    }

    ~Tile() {
      SDL_DestroyTexture(texture);
    }

    void render(SDL_Renderer *r, SDL_Rect *rect) {
      SDL_RenderCopy(r, texture, nullptr, rect);
    }
};

class Map {
  private:
    Tile **map;

    uint32_t width;
    uint32_t height;
    uint32_t tileSize;

  public:
    Map(SDL_Renderer *r, uint32_t w, uint32_t h, uint32_t tileSize = 16)
      : width(w)
      , height(h)
      , tileSize(tileSize)
      , map { new Tile * [w * h] }
    {
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          if (x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1) {
            map[y * width + x] = new Tile(r, "wall");
          } else {
            map[y * width + x] = new Tile(r, "floor");
          }
        }
      }
    }

    ~Map() {
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          delete map[y * width + x];
        }
      }

      delete [] map;
    }

    Tile * get(uint32_t x, uint32_t y) {
      return map[y * width + x];
    }

    void render(uint32_t ox, uint32_t oy, SDL_Renderer *r) {
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          SDL_Rect rect {
            static_cast<int>(ox + x * tileSize),
            static_cast<int>(oy + y * tileSize),
            static_cast<int>(tileSize),
            static_cast<int>(tileSize),
          };

          get(x, y)->render(r, &rect);
        }
      }
    }
};

class Player {
  private:
    SDL_Texture *texture;

  public:
    uint32_t x;
    uint32_t y;

    Player(SDL_Renderer *r, uint32_t x, uint32_t y)
      : x(x)
      , y(y)
      , texture { loadTexture(r, "res/player.png") }
    { }

    void render(SDL_Renderer *r, uint32_t tileSize) {
      SDL_Rect rect {
        static_cast<int>(x * tileSize),
        static_cast<int>(y * tileSize),
        static_cast<int>(tileSize),
        static_cast<int>(tileSize),
      };

      SDL_RenderCopy(r, texture, nullptr, &rect);
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
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (renderer == nullptr) {
    printf("Failed to create rederer.\nSDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);

  /* Set up text rendering */
  TTF_Font *font = TTF_OpenFont("res/PxPlus_IBM_VGA8.ttf", 28);

  if (font == nullptr) {
    printf("Failed to load font.\nTTF_Error: %s\n", TTF_GetError());
    return 1;
  }

  /* Data */
  Map m(renderer, 20, 20);
  Player p(renderer, 5, 5);

  /* Main loop */
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
        case SDL_QUIT:
          quit = true;
          break;

        case SDL_KEYDOWN:
          switch (e.key.keysym.sym) {
            case SDLK_LEFT:  p.x--; break;
            case SDLK_RIGHT: p.x++; break;
            case SDLK_UP:    p.y--; break;
            case SDLK_DOWN:  p.y++; break;
          }
          break;
      }
    }

    SDL_RenderClear(renderer);

    m.render(0, 0, renderer);
    p.render(renderer, 16);

    SDL_RenderPresent(renderer);
  }

  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}
