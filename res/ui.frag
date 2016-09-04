#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D tex;
uniform vec2 position;
uniform vec2 size;

void main() {
  color = texture(tex, UV); //vec4(0.0, 0.0, 0.0, 0.5);
}
