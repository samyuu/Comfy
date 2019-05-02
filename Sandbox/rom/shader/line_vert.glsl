#version 420 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;

out vec4 vertexColor;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(in_position, 1.0);
	vertexColor = in_color;
}