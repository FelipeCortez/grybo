#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in float fogIntensity;

uniform sampler2D texture2d;
uniform vec4 noteColor;

void main() {
  float texMask = 1.0f - step(0.333f, texCoord.x); // gets just the first pixel (1/3 of the texture. lol)
  vec4 color = vec4(0x74 / 255.0f, 0xb5 / 255.0f, 0x44 / 255.0f, 1.0f);
  vec4 colorMask = mix(texture(texture2d, texCoord), noteColor, texMask);

  FragColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f),
                  texture(texture2d, texCoord) * colorMask,
                  fogIntensity);
}
