#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 tileSize;
uniform mat4 projection;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 uv;

void main() {
  uv = texCoords;
  gl_Position = projection * tileSize * view * model * vec4(position, 0.0, 1.0);
}
