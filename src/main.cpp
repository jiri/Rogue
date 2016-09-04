#include <cstdio>
#include <cstdint>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

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

    virtual void render(GraphicsContext context) const = 0;
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
          0.0f, -0.5f,  0.0f, 0.0f,
          0.0f,  1.0f,  0.0f, 1.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
                 
          0.0f, -0.5f,  0.0f, 0.0f,
          1.0f, -0.5f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
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

class Player : public OrientedEntity {
  private:
    GLuint texture;
    int textureWidth, textureHeight;

    GLuint vao, vbo;
    std::vector<GLfloat> verticesN;
    std::vector<GLfloat> verticesE;
    std::vector<GLfloat> verticesS;
    std::vector<GLfloat> verticesW;

  public:
    Player(uint32_t x, uint32_t y)
      : OrientedEntity(x, y, false)
    {
      /* Create the texture */
      glGenTextures(1, &texture);

      glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint8_t *image = SOIL_load_image("res/player.png", &textureWidth, &textureHeight, 0, SOIL_LOAD_RGBA);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);
      glBindTexture(GL_TEXTURE_2D, 0);
      
      /* Create the model */
      verticesN.insert(verticesN.end(), {
          0.0f, -0.5f,  0.0f, 0.0f,
          0.0f,  1.0f,  0.0f, 1.0f,
          1.0f,  1.0f,  .25f, 1.0f,
                 
          0.0f, -0.5f,  0.0f, 0.0f,
          1.0f, -0.5f,  .25f, 0.0f,
          1.0f,  1.0f,  .25f, 1.0f,
      });
      verticesE.insert(verticesE.end(), {
          0.0f, -0.5f,  .25f, 0.0f,
          0.0f,  1.0f,  .25f, 1.0f,
          1.0f,  1.0f,  0.5f, 1.0f,
                 
          0.0f, -0.5f,  .25f, 0.0f,
          1.0f, -0.5f,  0.5f, 0.0f,
          1.0f,  1.0f,  0.5f, 1.0f,
      });
      verticesS.insert(verticesS.end(), {
          0.0f, -0.5f,  0.5f, 0.0f,
          0.0f,  1.0f,  0.5f, 1.0f,
          1.0f,  1.0f,  .75f, 1.0f,
                 
          0.0f, -0.5f,  0.5f, 0.0f,
          1.0f, -0.5f,  .75f, 0.0f,
          1.0f,  1.0f,  .75f, 1.0f,
      });
      verticesW.insert(verticesW.end(), {
          0.0f, -0.5f,  .75f, 0.0f,
          0.0f,  1.0f,  .75f, 1.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
                 
          0.0f, -0.5f,  .75f, 0.0f,
          1.0f, -0.5f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f,
      });

      /* Generate the VAO */
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

      /* Position attribute */
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(0);

      /* Color attribute */
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
      glEnableVertexAttribArray(1);

      glBindVertexArray(0);
    }

    bool interact() override {
      return false;
    };

    void render(GraphicsContext context) const override {
      /* FIXME: HAHA EWWW */

      context.model *= translate(vec3(position.x, position.y, 0));
      context.updateContext();

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      switch (orientation) {
        case N:
          glBufferData(GL_ARRAY_BUFFER, verticesN.size() * sizeof(GLfloat), verticesN.data(), GL_STATIC_DRAW);
          break;                                                                 
        case E:                                                                  
          glBufferData(GL_ARRAY_BUFFER, verticesE.size() * sizeof(GLfloat), verticesE.data(), GL_STATIC_DRAW);
          break;                                                                 
        case S:                                                                  
          glBufferData(GL_ARRAY_BUFFER, verticesS.size() * sizeof(GLfloat), verticesS.data(), GL_STATIC_DRAW);
          break;                                                                 
        case W:                                                                  
          glBufferData(GL_ARRAY_BUFFER, verticesW.size() * sizeof(GLfloat), verticesW.data(), GL_STATIC_DRAW);
          break;
      }

      glBindVertexArray(vao);      
      glBindTexture(GL_TEXTURE_2D, texture);

      glDrawArrays(GL_TRIANGLES, 0, verticesN.size());

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
    std::vector<Entity *> entities;

    uint32_t width;
    uint32_t height;

    GLuint texture;
    GLuint framebuffer;

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

      entities.push_back(new Obelisk { 5, 5 });
      entities.push_back(new Obelisk { 7, 7 });

      /* Generate the model */
      vertices.insert(vertices.end(), {
          0.0f,     0.0f,      1.0f, 1.0f,
          (float)w, (float)h,  0.0f, 0.0f,
          0.0f,     (float)h,  1.0f, 0.0f,

          (float)w, (float)h,  0.0f, 0.0f,
          0.0f,     0.0f,      1.0f, 1.0f,
          (float)w, 0.0f,      0.0f, 1.0f,

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

      Shader s("res/simple.vsh", "res/simple.fsh");

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

    void renderEntities(GraphicsContext context) const {
      for (auto & e : entities) {
        e->render(context);
      }
    }

    void addEntity(Entity * e) {
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

    Entity * getEntity(vec2 pos) {
      for (auto e : entities) {
        if (e->position == pos) {
          return e;
        }
      }

      return nullptr;
    }
};

class Camera {
  private:
    vec2 position;
    const Entity &target;

  public:
    Camera(const Entity & e)
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

class OrientedEntityController {
  private:
    OrientedEntity &entity;
    Map &map;

  public:
    OrientedEntityController(OrientedEntity &e, Map &m)
      : entity(e)
      , map(m)
    { }

    bool handleKey(int key) {
      vec2 delta;

      if (key == GLFW_KEY_UP) {
        entity.orientation = N;
        delta.y = -1;
      }
      if (key == GLFW_KEY_RIGHT) {
        entity.orientation = E;
        delta.x = 1;
      }
      if (key == GLFW_KEY_DOWN) {
        entity.orientation = S;
        delta.y = 1;
      }
      if (key == GLFW_KEY_LEFT) {
        entity.orientation = W;
        delta.x = -1;
      }
      

      if (key == GLFW_KEY_SPACE) {
        switch (entity.orientation) {
          case N: delta.y = -1; break;
          case E: delta.x =  1; break;
          case S: delta.y =  1; break;
          case W: delta.x = -1; break;
        }

        auto e = map.getEntity(entity.position + delta);

        if (e != nullptr) {
          e->interact();
        }

        return true;
      }

      if (map.passable(entity.position + delta)) {
        entity.position += delta;
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

struct Character {
  GLuint     textureID; // ID handle of the glyph texture
  ivec2 size;      // Size of glyph
  ivec2 bearing;   // Offset from baseline to left/top of glyph
  GLuint     advance;   // Offset to advance to next glyph
};

std::vector<Character> characters;

void renderText(Shader &s, std::string text, GLfloat x, GLfloat y, vec3 color = vec3(0), GLfloat scale = 1.0) {
  /* Set up the vertices */
  GLuint vao, vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);  

  /* Activate corresponding shader */
  s.use();

  s.setUniform("textColor", color);
  s.setUniform("projection", ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f));

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vao);

  /* Iterate through all characters */
  for (auto c = text.begin(); c != text.end(); c++) {
    Character ch = characters[*c];

    GLfloat xpos = x + ch.bearing.x * scale;
    GLfloat ypos = y - ch.bearing.y * scale;

    GLfloat w = ch.size.x * scale;
    GLfloat h = ch.size.y * scale;

    /* Update VBO for each character */
    GLfloat vertices[24] = {
      xpos,     ypos + h,   0.0, 1.0,            
      xpos,     ypos,       0.0, 0.0,
      xpos + w, ypos,       1.0, 0.0,

      xpos,     ypos + h,   0.0, 1.0,
      xpos + w, ypos,       1.0, 0.0,
      xpos + w, ypos + h,   1.0, 1.0,          
    };

    /* Render glyph texture over quad */
    glBindTexture(GL_TEXTURE_2D, ch.textureID);

    /* Update content of VBO memory */
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Render quad */
    glDrawArrays(GL_TRIANGLES, 0, 6);

    x += (ch.advance >> 6) * scale;
  }

  s.disuse();

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
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
  FT_Face face;

  if (FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Failed to initialize FreeType.\n");
    return -1;
  }

  if (FT_New_Face(ft, "res/Minecraftia.ttf", 0, &face)) {
    fprintf(stderr, "Failed to load font 'res/PxPlus_IBM_VGA8.ttf'.\n");
    return -1;
  }

  FT_Set_Pixel_Sizes(face, 0, 8);

  /* Generate the charset */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
  
  for (GLubyte c = 0; c < 128; c++) {
    /* Load character glyph */
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      fprintf(stderr, "Failed to load glyph 0x%x.\n", c);
      continue;
    }

    /* Generate texture */
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

    /* Set texture options */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* Now store character for later use */
    Character character = {
        texture, 
        ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        (GLuint) face->glyph->advance.x
    };

    characters.push_back(character);
  }

  /* Clean up after FreeType */
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  /* Data */
  TileSet t("res/tiles.png");
  Map m(20, 20, t);

  Player player { 1, 2 };
  OrientedEntityController pc { player, m };

  m.addEntity(&player);

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

  Shader text("res/text.vert", "res/text.frag");
  text.setUniform("projection", projection);

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

    renderText(text, "Hello, world! :p", 12, 24, vec3(1));

    glfwSwapBuffers(window);
  }

  /* Cleanup */
  glfwTerminate();
  return 0;
}
