#version 420 core
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texture_coords;
layout (location = 2) in vec4 in_color;

out vec4 vertexColor;
out vec2 vertexTexCoord;

uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * vec4(in_position, 0.0, 1.0);
	vertexColor = in_color;
	vertexTexCoord = vec2(in_texture_coords.x, 1.0 - in_texture_coords.y);
}