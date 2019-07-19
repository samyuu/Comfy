#version 420 core
layout (location = 0) in vec2 in_Position;
layout (location = 1) in vec2 in_TextureCoords;

out vec2 VertexTexCoord;

void main()
{
    gl_Position = vec4(in_Position.xy, 0.0, 1.0); 
    VertexTexCoord = in_TextureCoords;
}  