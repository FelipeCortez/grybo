#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;
out float fogIntensity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  texCoord = vec2(aTexCoord.x, aTexCoord.y);
  fogIntensity = clamp(1 - ((gl_Position.z - 3.0f) / 10.0f), 0.0f, 1.0f);
}
