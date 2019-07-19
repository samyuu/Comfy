#version 420 core
out vec4 FragColor;
  
in vec2 VertexTexCoord;

uniform sampler2D u_ScreenTexture;

void main()
{ 
	FragColor = texture(u_ScreenTexture, VertexTexCoord);
}