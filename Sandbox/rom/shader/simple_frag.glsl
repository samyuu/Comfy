#version 420 core
out vec4 FragColor;

in vec3 VertexPosition;
in vec3 VertexNormal;

uniform sampler2D u_TextureSampler0;
uniform sampler2D u_TextureSampler1;

const vec3 AmbientColor = vec3(0.1, 0.0, 0.00);
const vec3 LightPosition = vec3(0, -1000, 0);
const vec3 LightColor = vec3(0.90, 0.8, 0.60);

void main()
{
	vec3 norm = normalize(VertexNormal);
	vec3 lightDir = normalize(LightPosition - VertexPosition);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColor;

	FragColor = vec4(AmbientColor + diffuse, 1.0);

	// FragColor = AmbientColor;
	// FragColor = vec4(VertexPosition.rgb, 0.5) * VertexNormal;
	// FragColor = VertexNormal;
}