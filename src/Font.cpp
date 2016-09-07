#include <Font.h>

namespace {
  const int SCREEN_WIDTH  = 640;
  const int SCREEN_HEIGHT = 480;
}

Font::Font(FT_Library ft, std::string path)
  : shader("res/text.vert", "res/text.frag")
{
  /* Load the face */
  FT_Face face;

  if (FT_New_Face(ft, path.c_str(), 0, &face)) {
    throw runtime_error("Failed to load font '" + path + "'.");
  }

  FT_Set_Pixel_Sizes(face, 0, 8);

  /* Generate the charset */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

  /* Prepare vertex arrays */
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
}

void Font::render(string text, vec2 position, vec4 color, float scale) {
  /* Activate corresponding shader */
  shader.use();

  shader.setUniform("textColor", color);
  shader.setUniform("projection", ortho(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f));

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vao);

  /* Iterate through all characters */
  for (auto c = text.begin(); c != text.end(); c++) {
    Character ch = characters[*c];

    GLfloat xpos = position.x + ch.bearing.x * scale;
    GLfloat ypos = position.y - ch.bearing.y * scale;

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

    position.x += (ch.advance >> 6) * scale;
  }

  shader.disuse();

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
