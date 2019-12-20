#ifndef DEBUGINTERFACE_HLSL
#define DEBUGINTERFACE_HLSL

#ifdef COMFY_VS
// NOTE: Vertex inputs:
#define a_position          input.Position
#define a_color             input.Color
#define a_normal            input.Normal
#define a_tangent           input.Tangent
#define a_tex0              input.TexCoord
#define a_tex1              input.TexCoordAmbient

// NOTE: Vertex outputs:
#define o_position          output.Position
#define o_color_f0          output.Color
#define o_color_f1          output.ColorSecondary
#define o_tex0              output.TexCoord
#define o_tex1              output.TexCoordAmbient
#define o_fog               output.FogFactor
#define o_tex_shadow0       output.TexCoordShadow
#define o_normal            output.Normal
#define o_tangent           output.Tangent
#define o_binormal          output.Binormal
#define o_eye               output.EyeDirection
#define o_reflect           output.Reflection
#endif /* COMFY_VS */

#ifdef COMFY_PS
// NOTE: Fragment inputs:
#define a_color0            input.Color
#define a_color1            input.ColorSecondary
#define a_tex0              input.TexCoord
#define a_tex_color0        input.TexCoord
#define a_tex_color1        input.TexCoordAmbient
#define a_tex_normal0       input.TexCoord
#define a_tex_specular      input.TexCoord
#define a_fogcoord          float2(input.FogFactor, 0.0)
#define a_eye               input.EyeDirection
#define a_normal            input.Normal
#define a_tangent           input.Tangent
#define a_binormal          input.Binormal
#define a_reflect           input.Reflection
#define fragment_position   input.Position

// NOTE: Fragment output:
#define o_color         outputColor
#endif /* COMFY_PS */

// NOTE: General
#define FLOAT1_ZERO                 ((float1)0)
#define FLOAT2_ZERO                 ((float2)0)
#define FLOAT3_ZERO                 ((float3)0)
#define FLOAT4_ZERO                 ((float4)0)
#define FLOAT1_ONE                  ((float1)1)
#define FLOAT2_ONE                  ((float2)1)
#define FLOAT3_ONE                  ((float3)1)
#define FLOAT4_ONE                  ((float4)1)

// NOTE: Program environment:
#define mvp                         (transpose(CB_ModelViewProjection))
#define mv                          (transpose(CB_ModelView))
#define camera_mvi                  (CB_Scene.View)
#define model_mtx                   (CB_Model)
#define model_mtx_it                (transpose(CB_Model))
// TODO:
#define nt_mtx                      (float3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
// TODO:
#define p_lit_luce                  (CB_Scene.LightColor)
// TODO:
#define p_texcol_coef               (float4(0.9, 0.0, 0.0, 0.0))
#define p_blend_color               (float4(1.0, 1.0, 1.0, 1.0))
#define p_offset_color              (float4(0.0, 0.0, 0.0, 1.0))
#define p_max_alpha                 (float4(1.0, 1.0, 1.0, 1.0))
#define p_fres_coef                 (CB_Material.FresnelCoefficient)
#define p_bump_depth                (CB_Material.BumpDepth)
// TODO: Should this be the same as p_fres_coef (?)
#define fres_coef                   (CB_Material.FresnelCoefficient)
#define p_fb_isize                  (CB_Scene.TexelRenderResolution)
// TODO:
#define program_env_00              (float4(42.00, 42.00, 75.00, 1.0))
// TODO:
#define program_env_17              (float4(0.06, 0.05, 0.88, 1.0))
// TODO:
#define program_env_19              (float4(0.0, 0.0, 0.0, 0.0))
// TODO:
#define p_reflect_refract_uv_scale  (float4(0.0, 0.0, 0.0, 0.0))
#define irrad_r                     (CB_Scene.IrradianceRed)
#define irrad_g                     (CB_Scene.IrradianceGreen)
#define irrad_b                     (CB_Scene.IrradianceBlue)
#define lit_dir                     (CB_Scene.StageLight.Direction)
#define lit_dir_w                   (CB_Scene.StageLight.Direction)
#define lit_diff                    (CB_Scene.LightColor)
#define lit_spec                    ((CB_Scene.StageLight.Specular.rgb * CB_Scene.LightColor.rgb) * (1.0 / (1.0 - cos(PI / 10.0))))
#define p_lit_dir                   (CB_Scene.CharacterLight.Direction)

// NOTE: State fog:
#define state_fog_params            (CB_Scene.DepthFog.Parameters)
#define p_fog_color                 (CB_Scene.DepthFog.Color)

// NOTE: State light:
#define state_light0_ambient        (CB_Scene.CharacterLight.Ambient)
#define state_light0_diffuse        (CB_Scene.CharacterLight.Diffuse)
#define state_light0_specular       (CB_Scene.CharacterLight.Specular)
#define state_light1_ambient        (CB_Scene.StageLight.Ambient)
#define state_light1_diffuse        (CB_Scene.StageLight.Diffuse)
#define state_light1_specular       (CB_Scene.StageLight.Specular)
#define state_lightprod1_ambient    (CB_Material.Specular * CB_Scene.StageLight.Ambient)
#define state_lightprod1_diffuse    (CB_Material.Diffuse * CB_Scene.StageLight.Diffuse)
#define state_lightprod1_specular   (CB_Material.Specular * CB_Scene.StageLight.Specular)

// NOTE: State material:
#define state_matrix_texture0       (CB_Material.DiffuseTextureTransform)
#define state_matrix_texture1       (CB_Material.AmbientTextureTransform)
// TODO:
#define state_matrix_texture6       (float4x4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0))
#define state_material_diffuse      (CB_Material.Diffuse)
#define state_material_ambient      (CB_Material.Ambient)
#define state_material_specular     (CB_Material.Specular)
#define state_material_emission     (CB_Material.Emission)
#define state_material_shininess    (CB_Material.Shininess)

// NOTE: Instructions:
#define TEMP                        float4

#define MOV(result, a)              result      =         ( (a) )
#define ADD(result, a, b)           result      =         ( (a) + (b) )
#define ADD_SAT(result, a, b)       result      = saturate( (a) + (b) )
#define SUB(result, a, b)           result      =         ( (a) - (b) )
#define SUB_SAT(result, a, b)       result      = saturate( (a) - (b) )
#define MUL(result, a, b)           result      =         ( (a) * (b) )
#define MUL_SAT(result, a, b)       result      = saturate( (a) * (b) )
#define MAD(result, a, b, c)        result      =         ( mad( (a), (b), (c) ) )
#define MAD_SAT(result, a, b, c)    result      = saturate( mad( (a), (b), (c) ) )
#define MIN(result, a, b)           result      =         ( min( (a), (b) ) )
#define MAX(result, a, b)           result      =         ( max( (a), (b) ) )
#define POW(result, a, b)           result      =         ( pow( (a), (b) ) )
#define LRP(result, a, b, c)        result      =         ( lerp( (c), (b), (a) ) )
#define RSQ(result, a)              result      =         ( rsqrt( (a) ) )
#define DP3(result, a, b)           result.x    =         ( dot( (a).xyz, (b).xyz ) )
#define DP3_SAT(result, a, b)       result.x    = saturate( dot( (a).xyz, (b).xyz ) )
#define DP4(result, a, b)           result.x    =         ( dot( (a).xyzw, (b).xyzw ) )
#define DP4_SAT(result, a, b)       result.x    = saturate( dot( (a).xyzw, (b).xyzw ) )
#define XPD(result, a, b)           result.xyz  =         ( cross( (a).xyz, (b).xyz ) )
#define NRM(result, a)              result.xyz  =         ( normalize( (a).xyz ) )
#define NRMH(result, a)             result.xyzw =         ( normalize( (a).xyzw ) )

#define TEX2D_00(result, texCoord)  result = DiffuseTexture.Sample(DiffuseSampler, (texCoord).xy)
#define TEX2D_01(result, texCoord)  result = AmbientTexture.Sample(AmbientSampler, (texCoord).xy)
#define TEX2D_02(result, texCoord)  result = NormalTexture.Sample(NormalSampler, (texCoord).xy).xyzx
#define TEX2D_03(result, texCoord)  result = SpecularTexture.Sample(SpecularSampler, (texCoord).xy)
// TODO: simple_reflect...
#define TEX2D_15(result, texCoord)  result = ScreenReflectionTexture.Sample(ScreenReflectionSampler, (texCoord).xy)
// TODO: ...
#define TEX2D_16(result, texCoord)  result = float4(0.0, 0.0, 0.0, 0.0)

#define TEXCUBE_05(result, texCoord) result = ReflectionCubeMap.Sample( ReflectionSampler, (texCoord).xyz )
#define TEXCUBE_09(result, texCoord) result = CharacterLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_10(result, texCoord) result = SunLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_11(result, texCoord) result = ReflectLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_12(result, texCoord) result = ShadowLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_13(result, texCoord) result = CharColorLightMap.Sample( LightMapSampler, (texCoord).xyz )

// NOTE: Debug:
#define DEBUG_RET_O_COLOR(color) return float4( (color).rgb, 1.0 );

// NOTE: Testing...
#define SET_VERTEX_POSITIONS \
    pos_m = a_position; \
    pos_v = mul(mul(pos_m, CB_Model), CB_Scene.View); \
    pos_c = mul(mul(pos_m, CB_Model), CB_Scene.ViewProjection); \
    pos_w = mul(mul(pos_m, CB_Model), CB_Scene.ViewProjection)

#define CHECK_CLIP_ALPHA_TEST if (FLAGS_ALPHA_TEST) ClipAlphaThreshold(o_color.a)

#endif /* DEBUGINTERFACE_HLSL */
