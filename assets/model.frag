#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in float fogIntensity;

uniform sampler2D texture2d;

void main() {
  FragColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f),
                  texture(texture2d, texCoord),
                  fogIntensity);
}