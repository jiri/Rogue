#pragma once

#include <string>
#include <stdexcept>

#include <GL/glew.h>

class Shader {
  private:
    static GLuint create_shader(GLenum type, const char *path);
    static GLuint create_program(const char *vertpath, const char *fragpath);

  public:
    GLuint id;

    Shader(const char *vertpath, const char *fragpath) {
      id = create_program(vertpath, fragpath);
    };

    void use() const {
      glUseProgram(id);
    };

    void disuse() const {
      glUseProgram(0);
    }

    GLuint location(std::string name) const {
      return glGetUniformLocation(id, name.c_str());
    };

    template <typename T>
    void setUniform(std::string name, const T &value) {
      throw std::invalid_argument { "I don't know how to set a uniform of that type." };
    }
};
