#version 420 core
out vec4 fragColor;

in vec4 vertexColor;
in vec2 vertexTexCoord;

uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;

void main()
{
	fragColor = vertexColor * mix(texture(textureSampler0, vertexTexCoord), texture(textureSampler1, vertexTexCoord), 0.5);
}