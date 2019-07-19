#version 420 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec4 in_Color;

out vec4 VertexColor;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.0);
	VertexColor = in_Color;
}