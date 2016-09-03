#version 330 core

uniform sampler2D t;

in vec2 uv;

layout (location = 0) out vec4 color;

void main() {
  color = texture(t, uv);
}
