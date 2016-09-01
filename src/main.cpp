#include <cstdio>
#include <cstdint>

#include <iostream>
#include <string>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Shader.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

#if 0
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

class Entity {
  public:
    uint32_t x;
    uint32_t y;

    bool passable;

    Entity(uint32_t x, uint32_t y, bool p)
      : x(x)
      , y(y)
      , passable(p)
    { }

    virtual ~Entity() { }

    virtual bool interact() = 0;

    virtual void render(uint32_t ox, uint32_t oy) const = 0;
};

enum Orientation { N = 0, E, S, W };

class OrientedEntity : public Entity {
  public:
    Orientation orientation;

    OrientedEntity(uint32_t x, uint32_t y, bool p, Orientation o = Orientation::N)
      : Entity(x, y, p)
      , orientation(o)
    { }

    virtual bool interact() = 0;

    virtual void render(uint32_t ox, uint32_t oy) const = 0;
};

class Obelisk : public Entity {
  private:
    SDL_Renderer *r;
    SDL_Texture *texture;

  public:
    Obelisk(uint32_t x, uint32_t y, SDL_Renderer *r)
      : Entity(x, y, false)
      , r(r)
    {
      texture = loadTexture(r, "res/obelisk.png");
    }

    ~Obelisk() {
      SDL_DestroyTexture(texture);
    }

    bool interact() override {
      printf("Stuff is inscribed in the stone in an acient script. You can't read it for shit.\n");
      return true;
    }

    void render(uint32_t ox, uint32_t oy) const override {
      SDL_Rect rect {
        static_cast<int>(ox + x * 16),
        static_cast<int>(oy + y * 16 - 8),
        static_cast<int>(16),
        static_cast<int>(24),
      };

      SDL_RenderCopy(r, texture, nullptr, &rect);
    }
};

class Player : public OrientedEntity {
  private:
    SDL_Renderer *r;
    SDL_Texture *texture;

  public:

    Player(SDL_Renderer *r, uint32_t x, uint32_t y)
      : OrientedEntity(x, y, false)
      , r(r)
      , texture { loadTexture(r, "res/player.png") }
    { }

    bool interact() override {
      return false;
    };

    void render(uint32_t ox, uint32_t oy) const override {
      SDL_Rect srcRect {
        static_cast<int>(orientation * 16),
        static_cast<int>(0),
        static_cast<int>(16),
        static_cast<int>(24),
      };

      SDL_Rect dstRect {
        static_cast<int>(ox),
        static_cast<int>(oy),
        static_cast<int>(16),
        static_cast<int>(24),
      };

      SDL_RenderCopy(r, texture, &srcRect, &dstRect);
    }
};

class OrientedEntityController {
  private:
    OrientedEntity &entity;
    Map &map;

  public:
    OrientedEntityController(OrientedEntity &e, Map &m)
      : entity(e)
      , map(m)
    { }

    bool processEvent(SDL_Event &e) {
      int dx = 0;
      int dy = 0;

      if (e.type != SDL_KEYDOWN) {
        return false;
      }

      switch (e.key.keysym.sym) {
        case SDLK_LEFT:  entity.orientation = W; dx = -1; break;
        case SDLK_RIGHT: entity.orientation = E; dx =  1; break;
        case SDLK_UP:    entity.orientation = N; dy = -1; break;
        case SDLK_DOWN:  entity.orientation = S; dy =  1; break;

        case SDLK_SPACE:
          switch (entity.orientation) {
            case N: dy = -1; break;
            case E: dx =  1; break;
            case S: dy =  1; break;
            case W: dx = -1; break;
          }

          auto e = map.getEntity(entity.x + dx, entity.y + dy);

          if (e != nullptr) {
            e->interact();
          }

          return true;
      }

      if (map.passable(entity.x + dx, entity.y + dy)) {
        entity.x += dx;
        entity.y += dy;
      }
      
      return true;
    }
};

class Camera {
  private:
    const Player &player;
    const Map &map;

    uint32_t x;
    uint32_t y;

  public:
    Camera(const Player &p, const Map &m)
      : player(p)
      , map(m)
      , x(0)
      , y(0)
    {
    }

    void render(SDL_Renderer *r) {
      map.render(
        SCREEN_WIDTH  / 4 - x,
        SCREEN_HEIGHT / 4 - y
      );
      player.render(
        SCREEN_WIDTH  / 4 - x + player.x * map.tileSet.tileSize,
        SCREEN_HEIGHT / 4 - y + player.y * map.tileSet.tileSize - 8
      );
    }

    void updatePosition(float delta) {
      int32_t dx = player.x * map.tileSet.tileSize - x;
      int32_t dy = player.y * map.tileSet.tileSize - y;

      if (dx < 0) {
        x = floor(x + delta * dx);
      } else {
        x = ceil(x + delta * dx);
      }

      if (dy < 0) {
        y = floor(y + delta * dy);
      } else {
        y = ceil(y + delta * dy);
      }
    }
};
#endif

struct Rect { GLfloat x, y, w, h; };

struct Tile {
  uint32_t id;
  bool passable;
};

class TileSet {
  public:
    GLuint texture;
    int textureWidth, textureHeight;

    uint32_t width, height;

    TileSet(std::string path, uint32_t w = 8, uint32_t h = 8)
      : width(w), height(h)
    {
      glGenTextures(1, &texture);

      glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint8_t *image = SOIL_load_image(path.c_str(), &textureWidth, &textureHeight, 0, SOIL_LOAD_RGBA);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);

        glGenerateMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~TileSet() {
      glDeleteTextures(1, &texture);
    }

    Rect tileRect(const Tile & tile) const {
      uint32_t x = tile.id % 8;
      uint32_t y = tile.id / 8;

      GLfloat tileWidth  = 1.0f / width;
      GLfloat tileHeight = 1.0f / height;

      return { x * tileWidth, y * tileHeight, tileWidth, tileHeight };
    }
};

class Map {
  public:
    const TileSet & tileSet;

    Tile *map;
    // std::vector<Entity *> entities;

    uint32_t width;
    uint32_t height;

    GLuint vao, vbo;
    std::vector<GLfloat> vertices;

    Map(uint32_t w, uint32_t h, const TileSet & t)
      : width(w)
      , height(h)
      , tileSet(t)
      , map { new Tile [w * h] }
    {
      /* Generate the map */
      for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
          if (x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1) {
            map[y * width + x] = Tile { 1, false };
          } else if (y == 1) {
            map[y * width + x] = Tile { 2, false  };
          } else {
            map[y * width + x] = Tile { 0, true  };
          }
        }
      }

      /* Generate the model */
      for (GLfloat y = 0; y < height; y++) {
        for (GLfloat x = 0; x < width; x++) {
          auto rect = tileSet.tileRect(get(x, y));

          // printf("Rect for %f %f:\n", x, y);
          // printf("  { %f %f %f %f }\n", rect.x, rect.y, rect.w, rect.h);

          vertices.insert(vertices.end(), {
              x + 0, y + 0, rect.x,          rect.y,
              x + 1, y + 1, rect.x + rect.w, rect.y + rect.h,
              x + 0, y + 1, rect.x,          rect.y + rect.h,

              x + 1, y + 1, rect.x + rect.w, rect.y + rect.h,
              x + 0, y + 0, rect.x,          rect.y,
              x + 1, y + 0, rect.x + rect.w, rect.y,
          });
        }
      }

      /* Generate the VAO */
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

      /* Position attribute */
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);

      /* Color attribute */
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
      glEnableVertexAttribArray(1);

      glBindVertexArray(0);

      // entities.push_back(new Obelisk { 5, 5 });
    }

    ~Map() {
      delete [] map;

      // for (auto e : entities) {
      //   delete e;
      // }
    }

    Tile & get(uint32_t x, uint32_t y) {
      return map[y * width + x];
    }

    const Tile & get(uint32_t x, uint32_t y) const {
      return map[y * width + x];
    }

    void render() const {
      glBindVertexArray(vao);      
      glBindTexture(GL_TEXTURE_2D, tileSet.texture);

      glDrawArrays(GL_TRIANGLES, 0, vertices.size());

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }

    bool passable(uint32_t x, uint32_t y) const {
      if (get(x, y).passable) {
        // for (auto e : entities) {
        //   if (e->x == x && e->y == y && e->passable == false) {
        //     return false;
        //   }
        // }

        return true;
      }

      return false;
    }

    // Entity * getEntity(uint32_t x, uint32_t y) {
    //   for (auto e : entities) {
    //     if (e->x == x && e->y == y) {
    //       return e;
    //     }
    //   }

    //   return nullptr;
    // }
};

// class Camera {
//   private:
//     // const Player &player;
//     // const Map &map;
// 
//     glm::vec2 position;
//     glm::vec2 target;
// 
//   public:
//     Camera(const Player &p, const Map &m)
//       : player(p)
//       , map(m)
//       , position(0, 0)
//       , target(0, 0)
//     { }
// 
//     void updatePosition(float delta) {
//       position += delta * (target - position);
//     }
// 
//     glm::mat4 viewMatrix() const {
//       return glm::translate(-position.x, -position.y, 0);
//     }
// };

class FPSCounter {
  private:
    double lastTime;
    int frames;

  public:
    FPSCounter()
      : lastTime { glfwGetTime() }
      , frames(0)
    { }

    void update() {
      double currentTime = glfwGetTime();
      frames++;

      if (currentTime - lastTime >= 1.0) {
        printf("%f ms/frame\n", 1000.0 / frames);

        frames = 0;
        lastTime = currentTime;
      }
    }
};

int main() {
  /* Initialize GLFW */
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  /* VSync on */
  glfwSwapInterval(1);

  /* Create the window */
  GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rogue", nullptr, nullptr);

  if (window == nullptr) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  /* Initialize GLEW */
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    printf("Failed to initialize GLEW.\n");
    return -1;
  }

  /* Set the viewport */
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

#if 0
  /* Data */
  Map m(20, 20, renderer, "tiles");
  Player p(renderer, 1, 2);

  OrientedEntityController controller { p, m };

  Camera c { p, m };
#endif

#if 0
  /* Main loop */
  bool quit = false;
  SDL_Event e;

  uint32_t time = SDL_GetTicks();

  while (!quit) {
    controller.processEvent(e);

    uint32_t current = SDL_GetTicks();
    float delta = (current - time) / 1000.0f;
    time = current;

    SDL_RenderClear(renderer);

    c.render(renderer);
    c.updatePosition(delta);

    SDL_RenderPresent(renderer);
  }
#endif

  Shader program("res/simple.vsh", "res/simple.fsh");

  TileSet t("res/tiles.png");
  Map m(20, 20, t);

  glm::mat4 projection = glm::ortho(
		0.0f,
		static_cast<float>(SCREEN_WIDTH),
		static_cast<float>(SCREEN_HEIGHT),
		0.0f);

  glm::mat4 model = glm::scale(glm::vec3(16,16,16));

  FPSCounter fps;

  while(!glfwWindowShouldClose(window)) {
    fps.update();

    glfwPollEvents();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    program.use();

    program.setUniform("projection", projection);

    m.render();

    program.disuse();

    glfwSwapBuffers(window);
  }

  /* Cleanup */
  glfwTerminate();
  return 0;
}
