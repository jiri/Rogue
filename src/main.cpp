/** FOOD FOR THOUGHT
  *
  * - Multiple entities occupying the same space
  * - Rewrite using composition over inheritance
  */

#include <cstdio>
#include <cstdint>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stdexcept>
using namespace std;

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Shader.h>
#include <Font.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

class GraphicsContext {
  private:
    Shader & shader;

  public:
    mat4 model;
    mat4 tileSize;
    mat4 view;
    mat4 projection;

    GraphicsContext(Shader & s, mat4 p, mat4 ts, mat4 v = mat4(), mat4 m = mat4())
      : shader(s)
      , model(m)
      , tileSize(ts)
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
      shader.setUniform("tileSize", tileSize);
      shader.setUniform("projection", projection);
    }
};

class Appearance {
  public:
    GLuint texture;

    GLuint vao, vbo, ebo;
    vector<GLfloat> vertices;
    vector<GLuint> elements;

  public:
    Appearance() {
      glGenTextures(1, &texture);
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);
    }

    virtual ~Appearance() {
      glDeleteTextures(1, &texture);
      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ebo);
    }

    void loadTexture(const string & path) {
      glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height;
        uint8_t *image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
};

class Window {
  protected:
    Appearance appearance;

    vec2 position;
    vec2 size;
    vec2 border;

    Shader shader;

  public:
    Window(const vec2 & p, const vec2 & s, const vec2 & b)
      : position(p)
      , size(s)
      , border(b)
      , shader("res/ui.vert", "res/ui.frag")
    {
      appearance.loadTexture("res/gui2.png");

      appearance.vertices = {
          p.x - border.x,           p.y - border.y,           0.0f, 0.0f,
          p.x,                      p.y - border.y,           .25f, 0.0f,
          p.x + size.x,             p.y - border.y,           .75f, 0.0f,
          p.x + size.x + border.x,  p.y - border.y,           1.0f, 0.0f,

          p.x - border.x,           p.y,                      0.0f, .25f,
          p.x,                      p.y,                      .25f, .25f,
          p.x + size.x,             p.y,                      .75f, .25f,
          p.x + size.x + border.x,  p.y,                      1.0f, .25f,

          p.x - border.x,           p.y + size.y,             0.0f, .75f,
          p.x,                      p.y + size.y,             .25f, .75f,
          p.x + size.x,             p.y + size.y,             .75f, .75f,
          p.x + size.x + border.x,  p.y + size.y,             1.0f, .75f,

          p.x - border.x,           p.y + size.y + border.y,  0.0f, 1.0f,
          p.x,                      p.y + size.y + border.y,  .25f, 1.0f,
          p.x + size.x,             p.y + size.y + border.y,  .75f, 1.0f,
          p.x + size.x + border.x,  p.y + size.y + border.y,  1.0f, 1.0f,
      };

      for (uint32_t x = 0; x < 3; x++) {
        for (uint32_t y = 0; y < 3; y++) {
          appearance.elements.insert(appearance.elements.end(), {
              (y + 0) * 4 + (x + 0),
              (y + 1) * 4 + (x + 1),
              (y + 0) * 4 + (x + 1),

              (y + 0) * 4 + (x + 0),
              (y + 1) * 4 + (x + 1),
              (y + 1) * 4 + (x + 0),
          });
        }
      }

      glBindVertexArray(appearance.vao);
        glBindBuffer(GL_ARRAY_BUFFER, appearance.vbo);
        glBufferData(GL_ARRAY_BUFFER, appearance.vertices.size() * sizeof(GLfloat), appearance.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, appearance.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, appearance.elements.size() * sizeof(GLuint), appearance.elements.data(), GL_STATIC_DRAW);

        /* Position attribute */
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        /* UV attribute */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
      glBindVertexArray(0);
    }

    void render() {
      shader.use();

      shader.setUniform("projection", ortho(0.0f, (float)SCREEN_WIDTH, (float) SCREEN_HEIGHT, 0.0f));
      shader.setUniform("position", position);
      shader.setUniform("size", size);

      glBindVertexArray(appearance.vao);
      glBindTexture(GL_TEXTURE_2D, appearance.texture);

      glDrawElements(GL_TRIANGLES, appearance.elements.size(), GL_UNSIGNED_INT, 0);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);

      shader.disuse();
    }
};

class LogWindow : public Window {
  private:
    std::vector<std::string> messages;
    uint32_t messageCount;

    Font & font;

  public:
    LogWindow(const vec2 & p, const vec2 & s, uint32_t mc, Font & f, const vec2 & b = vec2(8.0f, 8.0f))
      : Window(p, s, b)
      , messageCount(mc)
      , font(f)
    { }

    void render() {
      Window::render();

      if (messages.size() > messageCount) {
        messages.resize(messageCount);
      }

      for (uint32_t i = 0; i < messages.size(); i++) {
        font.render(
            messages[i],
            vec2(position.x + 2, position.y + size.y - i * 16 - 2),
            vec4(1.0f, 1.0f, 1.0f, 1.0f - (1.0f / messageCount) * i),
            1.5f
        );
      }
    }

    void log(std::string message) {
      messages.insert(messages.begin(), message);
    }
};

class Logger {
  public:
    static LogWindow * window;

    static void log(std::string message) {
      if (window != nullptr) {
        window->log(message);
      }
    }
};

LogWindow * Logger::window;

enum Orientation { N = 0, E, S, W };

class Item {
  public:
    string name;
    Appearance appearance;

    Item(string n)
      : name(n)
    {
      appearance.loadTexture("res/items.png");

      /* Create the model */
      appearance.vertices = {
          .125f,  .125f,  0.0f,  0.0f,
          .125f,  .875f,  0.0f,  0.1f,
          .875f,  .875f,  .125f, 0.1f,

          .125f,  .125f,  0.0f,  0.0f,
          .875f,  .125f,  .125f, 0.0f,
          .875f,  .875f,  .125f, 0.1f,
      };

      /* Generate the VAO */
      glBindVertexArray(appearance.vao);
        glBindBuffer(GL_ARRAY_BUFFER, appearance.vbo);
        glBufferData(GL_ARRAY_BUFFER, appearance.vertices.size() * sizeof(GLfloat), appearance.vertices.data(), GL_STATIC_DRAW);

        /* Position */
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        /* Texture coordinates */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
      glBindVertexArray(0);
    }
};

class Inventory {
  public:
    vector<unique_ptr<Item>> items;

    void addItem(unique_ptr<Item> && i) {
      items.push_back(move(i));
    }

    void log() const {
      Logger::log("You have:");
      for (auto & i : items) {
        Logger::log("a " + i->name);
      }
    }
};

class Subject;

class Observer {
  public:
    virtual void onNotify(Subject & s, uint32_t event) = 0;
};

class Subject {
  protected:
    vector<Observer *> observers;

  public:
    void addObserver(Observer * o) {
      observers.push_back(o);
    }

    void notify(uint32_t event) {
      for (auto * o : observers) {
        o->onNotify(*this, event);
      }
    }
};

class Actor {
  public:
    enum {
      EVENT_IMPLOSION
    };

    Subject events;

    Inventory inventory;
    Appearance appearance;

    vec2 position;

    bool passable;

    Actor(uint32_t x, uint32_t y, bool p)
      : position(x, y)
      , passable(p)
    { }

    virtual ~Actor() { }

    virtual void interact(Actor & other) = 0;

    virtual void render(GraphicsContext context) const = 0;

    void implode() {
      events.notify(EVENT_IMPLOSION);
    }
};

class OrientedActor : public Actor {
  public:
    Orientation orientation;

    OrientedActor(uint32_t x, uint32_t y, bool p, Orientation o = N)
      : Actor(x, y, p)
      , orientation(o)
    { }

    void turnTo(const Actor & e) {
      if (e.position.x < position.x) {
        orientation = W;
        return;
      }

      if (e.position.x > position.x) {
        orientation = E;
        return;
      }

      if (e.position.y < position.y) {
        orientation = N;
        return;
      }

      if (e.position.y > position.y) {
        orientation = S;
        return;
      }
    }
};

class Obelisk : public Actor {
  public:
    Obelisk(uint32_t x, uint32_t y)
      : Actor(x, y, false)
    {
      appearance.loadTexture("res/obelisk.png");
      
      /* Create the model */
      appearance.vertices = {
          0.0f, -0.5f,  0.0f, 0.0f,
          0.0f,  1.0f,  0.0f, 1.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
                 
          0.0f, -0.5f,  0.0f, 0.0f,
          1.0f, -0.5f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
      };

      /* Generate the VAO */
      glBindVertexArray(appearance.vao);
        glBindBuffer(GL_ARRAY_BUFFER, appearance.vbo);
        glBufferData(GL_ARRAY_BUFFER, appearance.vertices.size() * sizeof(GLfloat), appearance.vertices.data(), GL_STATIC_DRAW);

        /* Position attribute */
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        /* Color attribute */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
      glBindVertexArray(0);
    }

    void interact(Actor &) override {
      Logger::log("Stuff is inscribed in the stone in an ancient script.");
      Logger::log("You can't read it for shit.");
    }

    void render(GraphicsContext context) const override {
      context.model *= translate(vec3(position.x, position.y, 0));
      context.updateContext();

      glBindVertexArray(appearance.vao);
      glBindTexture(GL_TEXTURE_2D, appearance.texture);

      glDrawArrays(GL_TRIANGLES, 0, appearance.vertices.size());

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }
};

class DroppedItem : public Actor {
  private:
    unique_ptr<Item> item;

  public:
    DroppedItem(uint32_t x, uint32_t y, Item *i)
      : Actor(x, y, true)
      , item(i)
    {
      appearance = item->appearance;
    }

    void interact(Actor & e) override {
      e.inventory.addItem(move(item));
      implode();
    }

    void render(GraphicsContext context) const override {
      context.model *= translate(vec3(position.x, position.y, 0));
      context.updateContext();

      glBindVertexArray(appearance.vao);
      glBindTexture(GL_TEXTURE_2D, appearance.texture);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }
};

class Chest : public OrientedActor {
  public:
    Chest(uint32_t x, uint32_t y, Orientation o = N)
      : OrientedActor(x, y, false, o)
    {
      /* Fill the inventory */
      inventory.addItem(make_unique<Item>("sword"));
      inventory.addItem(make_unique<Item>("sword"));

      /* Create the texture */
      appearance.loadTexture("res/chest.png");

      /* Create the model */
      for (float f = 0.0f; f < 1.0f; f += 0.25f) {
        appearance.vertices.insert(appearance.vertices.end(), {
            0.0f,  0.0f,  f,        0.0f,
            0.0f,  1.0f,  f,        1.0f,
            1.0f,  1.0f,  f + .25f, 1.0f,

            0.0f,  0.0f,  f,        0.0f,
            1.0f,  0.0f,  f + .25f, 0.0f,
            1.0f,  1.0f,  f + .25f, 1.0f,
        });
      }

      /* Generate the VAO */
      glBindVertexArray(appearance.vao);
        glBindBuffer(GL_ARRAY_BUFFER, appearance.vbo);
        glBufferData(GL_ARRAY_BUFFER, appearance.vertices.size() * sizeof(GLfloat), appearance.vertices.data(), GL_STATIC_DRAW);

        /* Position attribute */
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        /* Color attribute */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
      glBindVertexArray(0);
    }

    void interact(Actor & other) override {
      for (auto & i : inventory.items) {
        other.inventory.addItem(move(i));
      }

      inventory.items.clear();
    }

    void render(GraphicsContext context) const override {
      context.model *= translate(vec3(position.x, position.y, 0));
      context.updateContext();

      glBindVertexArray(appearance.vao);
      glBindTexture(GL_TEXTURE_2D, appearance.texture);

      glDrawArrays(GL_TRIANGLES, orientation * 6, 6);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }
};

class Player : public OrientedActor {
  public:
    Player(uint32_t x, uint32_t y)
      : OrientedActor(x, y, false)
    {
      /* Create the texture */
      appearance.loadTexture("res/player.png");
      
      /* Create the model */
      for (float f = 0.0f; f < 1.0f; f += 0.25f) {
        appearance.vertices.insert(appearance.vertices.end(), {
            0.0f, -0.5f,  f,        0.0f,
            0.0f,  1.0f,  f,        1.0f,
            1.0f,  1.0f,  f + .25f, 1.0f,

            0.0f, -0.5f,  f,        0.0f,
            1.0f, -0.5f,  f + .25f, 0.0f,
            1.0f,  1.0f,  f + .25f, 1.0f,
        });
      }

      /* Generate the VAO */
      glBindVertexArray(appearance.vao);
        glBindBuffer(GL_ARRAY_BUFFER, appearance.vbo);
        glBufferData(GL_ARRAY_BUFFER, appearance.vertices.size() * sizeof(GLfloat), appearance.vertices.data(), GL_STATIC_DRAW);

        /* Position */
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        /* Texture coordinates */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
      glBindVertexArray(0);
    }

    void interact(Actor & e) override {
      turnTo(e);
      Logger::log("Hello there!");
    }

    void render(GraphicsContext context) const override {
      context.model *= translate(vec3(position.x, position.y, 0));
      context.updateContext();

      glBindVertexArray(appearance.vao);
      glBindTexture(GL_TEXTURE_2D, appearance.texture);

      glDrawArrays(GL_TRIANGLES, orientation * 6, 6);

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

class Map : public Observer {
  public:
    const TileSet & tileSet;

    Tile *map;
    vector<Actor *> entities;

    uint32_t width;
    uint32_t height;

    GLuint texture;
    GLuint framebuffer;

    GLuint vao, vbo;
    vector<GLfloat> vertices;
    Shader s;

    Map(uint32_t w, uint32_t h, const TileSet & t)
      : width(w)
      , height(h)
      , tileSet(t)
      , map { new Tile [w * h] }
      , s("res/simple.vsh", "res/simple.fsh")
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

      addActor(new Obelisk { 5, 5 });
      addActor(new Chest { 7, 7, S});
      addActor(new Player { 5, 9 });
      addActor(new DroppedItem { 2, 2, new Item { "sword" } });

      /* Generate the model */
      auto fw = static_cast<float>(w);
      auto fh = static_cast<float>(h);

      vertices.insert(vertices.end(), {
          0.0f, 0.0f,  1.0f, 1.0f,
          fw,   fh,    0.0f, 0.0f,
          0.0f, fh,    1.0f, 0.0f,

          fw,   fh,    0.0f, 0.0f,
          0.0f, 0.0f,  1.0f, 1.0f,
          fw,   0.0f,  0.0f, 1.0f,

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

      /* Prepare the framebuffer */
      glGenFramebuffers(1, &framebuffer);
      glGenTextures(1, &texture);
    }

    ~Map() {
      delete [] map;

      // for (auto * a : entities) {
      //   delete a;
      // }

      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);

      glDeleteFramebuffers(1, &framebuffer);
      glDeleteTextures(1, &texture);
    }

    void renderMap() {
      int v[4];
      glGetIntegerv(GL_VIEWPORT, v);

      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

      glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width * 16, height * 16, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      glBindTexture(GL_TEXTURE_2D, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

      GLuint rbo;
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width * 16, height * 16);  
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);  

      GLuint vao, vbo;
      std::vector<GLfloat> vertices;

      for (GLfloat y = 0; y < height; y++) {
        for (GLfloat x = 0; x < width; x++) {
          auto rect = tileSet.tileRect(get(x, y));

          vertices.insert(vertices.end(), {
              (x + 0), (y + 0), rect.x,          rect.y,
              (x + 1), (y + 1), rect.x + rect.w, rect.y + rect.h,
              (x + 0), (y + 1), rect.x,          rect.y + rect.h,

              (x + 1), (y + 1), rect.x + rect.w, rect.y + rect.h,
              (x + 0), (y + 0), rect.x,          rect.y,
              (x + 1), (y + 0), rect.x + rect.w, rect.y,
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

      /* Render to the framebuffer */
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glViewport(0, 0, width * 16, height * 16);

      s.use();

      s.setUniform("model", mat4());
      s.setUniform("projection", ortho(0.0f, (float) width, (float) height, 0.0f));
      s.setUniform("view", mat4());
      s.setUniform("tileSize", mat4());

      glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glBindVertexArray(vao);      
      glBindTexture(GL_TEXTURE_2D, tileSet.texture);

      glDrawArrays(GL_TRIANGLES, 0, vertices.size());

      s.disuse();

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glViewport(v[0], v[1], v[2], v[3]);
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
      glBindTexture(GL_TEXTURE_2D, texture);

      glDrawArrays(GL_TRIANGLES, 0, vertices.size());

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindVertexArray(0);
    }

    void renderEntities(GraphicsContext context) {
      sort(entities.begin(), entities.end(), [](Actor * a, Actor * b) -> bool {
        return a->position.y < b->position.y;
      });

      for (auto & e : entities) {
        e->render(context);
      }
    }

    void addActor(Actor * e) {
      e->events.addObserver(this);
      entities.push_back(e);
    }

    bool passable(vec2 p) const {
      if (get(p.x, p.y).passable) {
        for (auto & e : entities) {
          if (e->position == p && e->passable == false) {
            return false;
          }
        }

        return true;
      }

      return false;
    }

    Actor * getActor(vec2 pos) {
      for (auto * e : entities) {
        if (e->position == pos) {
          return e;
        }
      }

      return nullptr;
    }

    void onNotify(Subject & a, uint32_t event) override {
      if (event == Actor::EVENT_IMPLOSION) {
        for (auto it = entities.begin(); it != entities.end(); it++) {
          if (&(*it)->events == &a) {
            Logger::log("Entity just died.");
            delete *it;
            entities.erase(it);
          }
        }
      }
    }
};

class Camera {
  private:
    vec2 position;
    const Actor &target;

  public:
    Camera(const Actor & e)
      : position(e.position)
      , target(e)
    { }

    void updatePosition(float delta) {
      position += delta * (target.position - position);
    }

    mat4 viewMatrix() const {
      return translate(vec3(-position.x, -position.y, 0));
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

    double delta() {
      double now = glfwGetTime();
      double d = now - lastTime;
      lastTime = now;
      return d;
    }
};

class OrientedActorController {
  private:
    OrientedActor &actor;
    Map &map;

  public:
    OrientedActorController(OrientedActor &e, Map &m)
      : actor(e)
      , map(m)
    { }

    bool handleKey(int key) {
      vec2 delta;

      if (key == GLFW_KEY_UP) {
        actor.orientation = N;
        delta.y = -1;
      }
      if (key == GLFW_KEY_RIGHT) {
        actor.orientation = E;
        delta.x = 1;
      }
      if (key == GLFW_KEY_DOWN) {
        actor.orientation = S;
        delta.y = 1;
      }
      if (key == GLFW_KEY_LEFT) {
        actor.orientation = W;
        delta.x = -1;
      }

      if (key == GLFW_KEY_SPACE) {
        switch (actor.orientation) {
          case N: delta.y = -1; break;
          case E: delta.x =  1; break;
          case S: delta.y =  1; break;
          case W: delta.x = -1; break;
        }

        auto e = map.getActor(actor.position + delta);

        if (e != nullptr) {
          e->interact(actor);
        }

        return true;
      }

      if (key == GLFW_KEY_TAB) {
        actor.inventory.log();
      }

      if (map.passable(actor.position + delta)) {
        actor.position += delta;
      }
      
      return true;
    }
};

std::queue<int> keys;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    keys.push(key);
  }
}

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

  glfwSetKeyCallback(window, key_callback);

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

  /* Initialize FreeType */
  FT_Library ft;

  if (FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Failed to initialize FreeType.\n");
    return -1;
  }

  /* Data */
  TileSet t("res/tiles.png");
  Map m(20, 20, t);

  Player player { 1, 2 };
  OrientedActorController pc { player, m };

  m.addActor(&player);

  Camera c { player };

  /* Shader & matrices */
  Shader program("res/simple.vsh", "res/simple.fsh");
  
  mat4 projection = ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f);
  mat4 center = translate(vec3(SCREEN_WIDTH / 64, SCREEN_HEIGHT / 64, 0));

  GraphicsContext context {
    program,
    projection,
    scale(vec3(32, 32, 32)), 
    center,
    mat4()
  };

  Font font(ft, "res/Denjuu-World.ttf");

  LogWindow l(vec2(12, SCREEN_HEIGHT - 12 - 144), vec2(396, 144), 9, font);
  Logger::window = &l;

  FPSCounter fps;
  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    /* Update camera */
    while (!keys.empty()) {
      pc.handleKey(keys.front());
      keys.pop();
    }

    c.updatePosition(fps.delta());

    /* Render the scene */
    m.renderMap();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    context.use();

    context.view = center * c.viewMatrix();
    context.updateContext();

    m.render(context);
    m.renderEntities(context);

    context.disuse();

    l.render();

    glfwSwapBuffers(window);
  }

  /* Cleanup */
  glfwTerminate();
  return 0;
}
