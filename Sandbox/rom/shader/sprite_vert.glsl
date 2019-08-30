#version 420 core
layout (location = 0) in vec2 in_Position;
layout (location = 1) in vec2 in_TextureCoords;
layout (location = 2) in vec2 in_TextureMaskCoords;
layout (location = 3) in vec4 in_Color;

out VS_OUT
{
	vec4 Color;
	vec2 TexCoord;
	vec2 TexMaskCoord;
} Output;

uniform mat4 u_ProjectionView;

void main()
{
	gl_Position = u_ProjectionView * vec4(in_Position, 0.0, 1.0);
	Output.Color = in_Color;
	Output.TexCoord = vec2(in_TextureCoords.x, 1.0 - in_TextureCoords.y);
	Output.TexMaskCoord = vec2(in_TextureMaskCoords.x, 1.0 - in_TextureMaskCoords.y);
}