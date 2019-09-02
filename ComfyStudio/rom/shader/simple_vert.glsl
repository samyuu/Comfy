#version 420 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;

out vec3 VertexPosition;
out vec3 VertexNormal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	VertexPosition = vec3(u_Model * vec4(in_Position, 1.0));
	VertexNormal = in_Normal;

	gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.0);
}