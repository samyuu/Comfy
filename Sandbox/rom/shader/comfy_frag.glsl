#version 420 core
out vec4 fragColor;

in vec4 vertexColor;
in vec2 vertexTexCoord;

uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;

const vec4 ambientColor = vec4(0.90, 0.8, 0.60, 1.0);

void main()
{
	// fragColor = vertexColor * texture(textureSampler0, vertexTexCoord);
	// fragColor = vertexColor * textureLod(textureSampler0, vertexTexCoord, 6);
	// fragColor = texture(textureSampler0, vertexTexCoord);
	// fragColor = vertexColor * mix(texture(textureSampler0, vertexTexCoord), texture(textureSampler1, vertexTexCoord), 0.5);

	fragColor = vertexColor * mix(texture(textureSampler0, vertexTexCoord), texture(textureSampler1, vertexTexCoord), 0.5) * ambientColor;
}