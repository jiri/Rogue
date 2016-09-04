#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
  color = vec4(textColor.rgb, texture(text, uv).r * textColor.a);
}
