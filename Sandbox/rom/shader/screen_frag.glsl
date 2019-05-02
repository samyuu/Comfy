#version 420 core
out vec4 fragColor;
  
in vec2 vertexTexCoord;

uniform sampler2D screenTexture;

void main()
{ 
	fragColor = texture(screenTexture, vertexTexCoord);
}