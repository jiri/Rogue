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

using namespace glm;

#include <Shader.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

#if 0
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
#endif

class GraphicsContext {
  private:
    Shader & shader;

  public:
    mat4 model;
    mat4 view;
    mat4 projection;

    GraphicsContext(Shader & s, mat4 p = mat4(), mat4 v = mat4(), mat4 m = mat4())
      : shader(s)
      , model(m)
      , view(v)
      , projection(p)
    { }

    void use() {
      shader.use();
    }

    void disuse() {
      shader.disuse();
    }

    void updateContext() {
      shader.setUniform("model", model);
      shader.setUniform("view", view);
      shader.setUniform("projection", projection);
    }
};

class Entity {
  public:
    vec2 position;

    bool passable;

    Entity(uint32_t x, uint32_t y, bool p)
      : position(x, y)
      , passable(p)
    { }

    virtual ~Entity() { }

    virtual bool interact() = 0;

    virtual void render(GraphicsContext context) const = 0;
};

enum Orientation { N = 0, E, S, W };

class OrientedEntity : public Entity {
  public:
    Orientation orientation;

    OrientedEntity(uint32_t x, uint32_t y, bool p, Orientation o = N)
      : Entity(x, y, p)
      , orientation(o)
    { }

    virtual bool interact() = 0;

    virtual void render() const = 0;
};

class Obelisk : public Entity {
  private:
    GLuint texture;
    int textureWidth, textureHeight;

    GLuint vao, vbo;
    std::vector<GLfloat> vertices;

  public:
    Obelisk(uint32_t x, uint32_t y)
      : Entity(x, y, false)
    {
      /* Create the texture */
      glGenTextures(1, &texture);

      glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint8_t *image = SOIL_load_image("res/obelisk.png", &textureWidth, &textureHeight, 0, SOIL_LOAD_RGBA);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);
      glBindTexture(GL_TEXTURE_2D, 0);
      
      /* Create the model */
      vertices.insert(vertices.end(), {
          0.0f, 0.0f,  0.0f, 0.0f,
          0.0f, 1.5f,  0.0f, 1.0f,
          1.0f, 1.5f,  1.0f, 1.0f,

          0.0f, 0.0f,  0.0f, 0.0f,
          1.0f, 0.0f,  1.0f, 0.0f,
          1.0f, 1.5f,  1.0f, 1.0f,
      });

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
    }

    ~Obelisk() {
      glDeleteTextures(1, &texture);

      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
    }

    bool interact() override {
      printf("Stuff is inscribed in the stone in an acient script. You can't read it for shit.\n");
      return true;
    }

    void render(GraphicsContext context) const override {
      context.model *= translate(vec3(position.x, position.y, 0));

      context.updateContext();

      glBindVertexArray(vao);      
      glBindTexture(GL_TEXTURE_2D, texture);

      glDrawArrays(GL_TRIANGLES, 0, vertices.size());

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }
};

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
    std::vector<std::unique_ptr<Entity>> entities;

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

      entities.push_back(std::unique_ptr<Obelisk>(new Obelisk { 5, 5 }));

      /* Generate the model */
      for (GLfloat y = 0; y < height; y++) {
        for (GLfloat x = 0; x < width; x++) {
          auto rect = tileSet.tileRect(get(x, y));

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
    }

    ~Map() {
      delete [] map;

      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
    }

    Tile & get(uint32_t x, uint32_t y) {
      return map[y * width + x];
    }

    const Tile & get(uint32_t x, uint32_t y) const {
      return map[y * width + x];
    }

    void render(GraphicsContext context) const {
      context.updateContext();

      glBindVertexArray(vao);      
      glBindTexture(GL_TEXTURE_2D, tileSet.texture);

      glDrawArrays(GL_TRIANGLES, 0, vertices.size());

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }

    void renderEntities(GraphicsContext context) const {
      for (auto & e : entities) {
        e->render(context);
      }
    }

    bool passable(uint32_t x, uint32_t y) const {
      if (get(x, y).passable) {
        for (auto & e : entities) {
          if (e->position == vec2(x, y) && e->passable == false) {
            return false;
          }
        }

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

class Camera {
  private:
    // const Player &player;
    // const Map &map;

    vec2 position;
    vec2 target;

  public:
    Camera(const vec2 & pos)
      : position(pos)
      , target(pos)
    { }

    Camera(uint32_t x, uint32_t y)
      : position(x, y)
      , target(x, y)
    { }

    Camera()
      : Camera(0, 0)
    { }

    void updatePosition(float delta) {
      position += delta * (target - position);
    }

    mat4 viewMatrix() const {
      return translate(vec3(-position.x, -position.y, 0));
    }

    void seek(uint32_t x, uint32_t y) {
      target = vec2(x, y);
    }
};

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

    double delta() const {
      return glfwGetTime() - lastTime;
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

  /* Enable transparency */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 0
  /* Data */
  Map m(20, 20, renderer, "tiles");
  Player p(renderer, 1, 2);

  OrientedEntityController controller { p, m };

  Camera c { p, m };
#endif

  /* Data */
  TileSet t("res/tiles.png");
  Map m(20, 20, t);

  Camera c(50, 50);

  /* Shader & matrices */
  Shader program("res/simple.vsh", "res/simple.fsh");

  mat4 projection = ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f);

  mat4 center = translate(vec3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0));

  mat4 model = scale(vec3(32, 32, 32));

  GraphicsContext context { program, projection, center, model };

  FPSCounter fps;
  while(!glfwWindowShouldClose(window)) {
    fps.update();

    glfwPollEvents();

    /* Update camera */
    c.updatePosition(fps.delta());

    static bool flag = true;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && flag) {
      c.seek(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
      flag = false;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
      flag = true;
    }

    /* Render the scene */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    context.use();

    context.view = center * c.viewMatrix();
    context.updateContext();

    m.render(context);
    m.renderEntities(context);

    context.disuse();

    glfwSwapBuffers(window);
  }

  /* Cleanup */
  glfwTerminate();
  return 0;
}
