#include <Shader.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

static std::string read_file(const char *path) {
  using namespace std;

  stringstream stream;

  ifstream file;
  file.exceptions(ifstream::badbit);

  try {
    file.open(path);
    stream << file.rdbuf();
    file.close();
  } catch(ifstream::failure e) {
    cerr << "Couldn't read file '" << path << "'." << endl;
  }

  return stream.str();
}

GLuint Shader::create_shader(GLenum type, const char *path) {
  /* Read the shader */
  auto contents = read_file(path);
  auto src = contents.c_str();

  /* Create the shader */
  GLuint shader_id = glCreateShader(type);

  glShaderSource(shader_id, 1, &src, nullptr);
  glCompileShader(shader_id);

  /* Check for errors */
  GLint result = GL_FALSE;
  int info_log_length = 0;

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

  if (info_log_length > 1) {
    using namespace std;

    char *error_message = (char *) calloc(info_log_length + 1, 1);
    glGetShaderInfoLog(shader_id, info_log_length, nullptr, error_message);

    cerr << "[" << result << "] Problem when compiling shader '" << path << "':" << endl;
    cerr << error_message << endl;

    free(error_message);
  }

  return shader_id;
}

GLuint Shader::create_program(const char *vertpath, const char *fragpath) {
  /* Create shaders */
  GLuint vert_shader = create_shader(GL_VERTEX_SHADER, vertpath);
  GLuint frag_shader = create_shader(GL_FRAGMENT_SHADER, fragpath);

  /* Link the program */
  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vert_shader);
  glAttachShader(program_id, frag_shader);
  glLinkProgram(program_id);

  /* Check for errors */
  GLint result = GL_FALSE;
  int info_log_length = 0;

  glGetProgramiv(program_id, GL_LINK_STATUS, &result);
  glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);

  if (info_log_length > 1) {
    using namespace std;

    char *error_message = (char *) calloc(info_log_length + 1, 1);
    glGetProgramInfoLog(program_id, info_log_length, nullptr, error_message);

    cerr << "Problem when linking shaders '" << vertpath << "', '" << fragpath << "':" << endl;
    cerr << error_message << endl;

    free(error_message);
  }

  /* Clean up */
  glDetachShader(program_id, vert_shader);
  glDetachShader(program_id, frag_shader);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  return program_id;
}

/* OpenGL primitives */
template <>
void Shader::setUniform<GLfloat>(std::string name, const GLfloat &value) {
  glUniform1f(location(name), value);
}

template <>
void Shader::setUniform<GLuint>(std::string name, const GLuint &value) {
  glUniform1ui(location(name), value);
}

template <>
void Shader::setUniform<GLint>(std::string name, const GLint &value) {
  glUniform1i(location(name), value);
}

/* GLM vectors */
template <>
void Shader::setUniform<vec2>(std::string name, const vec2 &value) {
  glUniform2f(location(name), value.x, value.y);
}

template <>
void Shader::setUniform<vec3>(std::string name, const vec3 &value) {
  glUniform3f(location(name), value.x, value.y, value.z);
}

template <>
void Shader::setUniform<vec4>(std::string name, const vec4 &value) {
  glUniform4f(location(name), value.x, value.y, value.z, value.w);
}

/* GLM matrices */
template <>
void Shader::setUniform<mat3>(std::string name, const mat3 &matrix) {
  glUniformMatrix3fv(location(name), 1, GL_FALSE, value_ptr(matrix));
}

template <>
void Shader::setUniform<mat4>(std::string name, const mat4 &matrix) {
  glUniformMatrix4fv(location(name), 1, GL_FALSE, value_ptr(matrix));
}
