#pragma once

#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Shader.h>

struct Character {
  GLuint textureID; /* ID handle of the glyph texture              */
  ivec2  size;      /* Size of glyph                               */
  ivec2  bearing;   /* Offset from baseline to left / top of glyph */
  GLuint advance;   /* Offset to advance to next glyph             */
};

class Font {
  private:
    vector<Character> characters;
    Shader shader;

    GLuint vao;
    GLuint vbo;

  public:
    Font(FT_Library ft, string path);

    void render(string text, vec2 position, vec4 color = vec4(0), float scale = 1.0f);
};
