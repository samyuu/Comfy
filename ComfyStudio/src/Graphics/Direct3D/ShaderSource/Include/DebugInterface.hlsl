#ifndef DEBUGINTERFACE_HLSL
#define DEBUGINTERFACE_HLSL

#ifdef COMFY_VS
// NOTE: Vertex inputs:
#define a_position input.Position
#define a_color input.Color
#define a_normal input.Normal
#define a_tangent input.Tangent
#define a_tex0 input.TexCoord
#define a_tex1 input.TexCoordAmbient

// NOTE: Vertex outputs:
#define o_position output.Position
#define o_color_f0 output.Color
#define o_color_f1 output.ColorSecondary
#define o_tex0 output.TexCoord
#define o_tex1 output.TexCoordAmbient
#define o_normal output.Normal
#define o_tangent output.Tangent
#define o_binormal output.Binormal
#define o_eye output.EyeDirection
#endif /* COMFY_VS */

#ifdef COMFY_PS
// NOTE: Fragment inputs:
#define a_color0 input.Color
#define a_color1 input.ColorSecondary
#define a_tex_color0 input.TexCoord
#define a_tex_color1 input.TexCoordAmbient
#define a_tex_normal0 input.TexCoord
#define a_tex_specular input.TexCoord
#define a_eye input.EyeDirection
#define a_normal input.Normal
#define a_tangent input.Tangent
#define a_binormal input.Binormal

// NOTE: Fragment output:
#define o_color outputColor
#endif /* COMFY_PS */

// NOTE: Params:
// #define mvp { state.matrix.mvp }
// #define mv { state.matrix.modelview }
// #define camera_mvi { state.matrix.program[5].inverse }
// #define model_mtx { state.matrix.program[6] }
// #define model_mtx_it { state.matrix.program[6].invtrans }

#define p_max_alpha float4(1.0, 1.0, 1.0, 1.0)
#define p_fres_coef CB_Material.FresnelCoefficient

#define irrad_r CB_Scene.IrradianceRed
#define irrad_g CB_Scene.IrradianceGreen
#define irrad_b CB_Scene.IrradianceBlue

#define lit_dir CB_Scene.StageLight.Direction
#define lit_diff CB_Scene.LightColor
#define lit_spec ((CB_Scene.StageLight.Specular.rgb * CB_Scene.LightColor.rgb) * (1.0 / (1.0 - cos(PI / 10.0))))

// NOTE: State light:
#define state_light1_ambient CB_Scene.StageLight.Ambient
#define state_light1_diffuse CB_Scene.StageLight.Diffuse
#define state_light1_specular CB_Scene.StageLight.Specular

// NOTE: State material:
#define state_matrix_texture0 CB_Material.DiffuseTextureTransform
#define state_matrix_texture1 CB_Material.AmbientTextureTransform

#define state_material_diffuse CB_Material.Diffuse
#define state_material_ambient CB_Material.Ambient
#define state_material_specular CB_Material.Specular
#define state_material_emission CB_Material.Emission
#define state_material_shininess CB_Material.Shininess

// NOTE: Instructions:
#define MOV(result, a) result = a

#define ADD(result, a, b) result = (a + b)
#define ADD_SAT(result, a, b) result = saturate(a + b)

#define SUB(result, a, b) result = (a - b)
#define SUB_SAT(result, a, b) result = saturate(a - b)

#define MUL(result, a, b) result = (a * b)
#define MAD(result, a, b, c) result = mad(a, b, c)

#define MAX(result, a, b) result = max(a, b)
#define POW(result, a, b) result = pow(a, b)
#define RSQ(result, a) result = rsqrt(a)

#define DP3(result, a, b) result = dot(a.xyz, b.xyz)
#define DP3_SAT(result, a, b) result = saturate(dot(a.xyz, b.xyz))
#define DP4(result, a, b) result = dot(a.xyzw, b.xyzw)
#define DP4_SAT(result, a, b) result = saturate(dot(a.xyzw, b.xyzw))

#define XPD(result, a, b) result = cross(a, b)

#define NRM(result, a) result.xyz = normalize(a.xyz)
#define NRMH(result, a) result = normalize(a)

#define TEX2D_00(result, texCoord) result = DiffuseTexture.Sample(DiffuseSampler, texCoord.xy)
#define TEX2D_01(result, texCoord) result = AmbientTexture.Sample(AmbientSampler, texCoord.xy)
#define TEX2D_02(result, texCoord) result = NormalTexture.Sample(NormalSampler, texCoord.xy).xyzx
#define TEX2D_03(result, texCoord) result = SpecularTexture.Sample(SpecularSampler, texCoord.xy)

#define TEXCUBE_05(result, texCoord) result = ReflectionCubeMap.Sample(ReflectionSampler, texCoord.xyz)
#define TEXCUBE_09(result, texCoord) result = CharacterLightMap.Sample(LightMapSampler, texCoord.xyz)
#define TEXCUBE_10(result, texCoord) result = SunLightMap.Sample(LightMapSampler, texCoord.xyz)
#define TEXCUBE_11(result, texCoord) result = ReflectLightMap.Sample(LightMapSampler, texCoord.xyz)
#define TEXCUBE_12(result, texCoord) result = ShadowLightMap.Sample(LightMapSampler, texCoord.xyz)
#define TEXCUBE_13(result, texCoord) result = CharColorLightMap.Sample(LightMapSampler, texCoord.xyz)

// NOTE: Debug:
#define DEBUG_RETURN_COLOR(color) return float4(color.rgb, 1.0);

#endif /* DEBUGINTERFACE_HLSL */
