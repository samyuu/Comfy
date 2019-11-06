#version 420 core
out vec4 FragColor;

in vec4 VertexColor;
in vec2 VertexTexCoord;

uniform sampler2D u_TextureSampler0;
uniform sampler2D u_TextureSampler1;

const vec4 AmbientColor = vec4(0.90, 0.8, 0.60, 1.0);

void main()
{
	FragColor = VertexColor * mix(texture(u_TextureSampler0, VertexTexCoord), texture(u_TextureSampler1, VertexTexCoord), 0.5) * AmbientColor;
}