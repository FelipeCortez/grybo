#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture2d;

void main()
{
	FragColor = texture(texture2d, TexCoord);
}
