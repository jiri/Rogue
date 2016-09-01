#version 330 core

uniform mat4 projection;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 uv;

void main() {
  mat4 scale;
  scale[0] = vec4(16, 0,    0,    0);
  scale[1] = vec4(0,    16, 0,    0);
  scale[2] = vec4(0,    0,    16, 0);
  scale[3] = vec4(0,    0,    0,    1);

  mat4 translate;
  translate[0] = vec4(1, 0, 0, 0);
  translate[1] = vec4(0, 1, 0, 0);
  translate[2] = vec4(0, 0, 1, 0);
  translate[3] = vec4(-0.5, 0.5, 0, 1);

  uv = texCoords;
  gl_Position = projection * scale * vec4(position, 0.0, 1.0);
}
