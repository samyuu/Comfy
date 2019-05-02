#version 420 core
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texture_coords;

out vec2 vertexTexCoord;

void main()
{
    gl_Position = vec4(in_position.xy, 0.0, 1.0); 
    vertexTexCoord = in_texture_coords;
}  