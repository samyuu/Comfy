#version 420 core
layout (location = 0) in vec2 in_Position;
layout (location = 1) in vec2 in_TextureCoords;
layout (location = 2) in vec2 in_TextureMaskCoords;
layout (location = 3) in vec4 in_Color;

out vec4 VertexColor;
out vec2 VertexTexCoord;
out vec2 VertexTexMaskCoord;

uniform mat4 u_ProjectionView;

void main()
{
	gl_Position = u_ProjectionView * vec4(in_Position, 0.0, 1.0);
	VertexColor = in_Color;
	
	VertexTexCoord = vec2(in_TextureCoords.x, 1.0 - in_TextureCoords.y);
	VertexTexMaskCoord = vec2(in_TextureMaskCoords.x, 1.0 - in_TextureMaskCoords.y);
}