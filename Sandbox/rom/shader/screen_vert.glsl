#version 420 core
out vec2 VertexTexCoord;

const vec4 VertexData[6] = 
{
	vec4(-1.0, +1.0, 0.0, 1.0),
	vec4(+1.0, +1.0, 1.0, 1.0),
	vec4(+1.0, -1.0, 1.0, 0.0),
	vec4(-1.0, +1.0, 0.0, 1.0),
	vec4(+1.0, -1.0, 1.0, 0.0),
	vec4(-1.0, -1.0, 0.0, 0.0),
};

void main()
{
    vec4 vertexData = VertexData[gl_VertexID];

    gl_Position = vec4(vertexData.xy, 0.0, 1.0);
    VertexTexCoord = vertexData.zw;
}  