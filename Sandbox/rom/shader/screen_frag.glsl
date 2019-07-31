#version 420 core
out vec4 FragColor;
  
in vec2 VertexTexCoord;

uniform float u_Saturation; // 2.8;
uniform float u_Brightness; // 0.455;
uniform sampler2D u_ScreenTexture;

vec4 Saturate(vec4 inputColor)
{
	const float almostZero = 0.0000000001;
	const vec3 saturation = vec3(u_Saturation);
	const vec3 brightness = vec3(u_Brightness);

	const vec3 RED = vec3(1.095, 0.0, 0.0);
	const vec3 GREEN = vec3(0.0, 1.0, 0.0);
	const vec3 BLUE = vec3(0.0, 0.0, 1.0);

	vec3 saturatedColor = vec3(
		exp2(log2(max(abs(inputColor.r), almostZero)) * saturation.r),
		exp2(log2(max(abs(inputColor.g), almostZero)) * saturation.g),
		exp2(log2(max(abs(inputColor.b), almostZero)) * saturation.b));

	return vec4(
		exp2(log2(max(abs(clamp(dot(saturatedColor, RED), 0.0, 1.0)), almostZero)) * brightness.r),
		exp2(log2(max(abs(clamp(dot(saturatedColor, GREEN), 0.0, 1.0)), almostZero)) * brightness.g),
		exp2(log2(max(abs(clamp(dot(saturatedColor, BLUE), 0.0, 1.0)), almostZero)) * brightness.b),
		inputColor.a);
}

void main()
{ 
	FragColor = texture(u_ScreenTexture, VertexTexCoord);
	FragColor = Saturate(FragColor);
}