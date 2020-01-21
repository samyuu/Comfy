#ifndef DEBUGINTERFACE_HLSL
#define DEBUGINTERFACE_HLSL

// --------------------------------------------------------------------------------------------------------------------------
#ifdef COMFY_VS
// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Vertex inputs:
#define a_position                  (float4(input.Position))
#define a_normal                    (float4(input.Normal, 1.0))
#define a_tangent                   (float4(input.Tangent))
#define a_tex0                      (float4(input.TexCoord, 0.0, 1.0))
#define a_tex1                      (float4(input.TexCoordAmbient, 0.0, 1.0))
#define a_color                     (float4(input.Color))
#define a_morph_position            (float4(input.MorphPosition))
#define a_morph_normal              (float4(input.MorphNormal, 1.0))
#define a_morph_tangent             (float4(input.MorphTangent))
#define a_morph_texcoord            (float4(input.MorphTexCoord, 0.0, 1.0))
#define a_morph_texcoord1           (float4(input.MorphTexCoordAmbient, 0.0, 1.0))
#define a_morph_color               (float4(input.MorphColor))
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Vertex outputs:
#define o_position                  (output.Position)
#define o_color_f0                  (output.Color)
#define o_color_f1                  (output.ColorSecondary)
#define o_tex0                      (output.TexCoord)
#define o_tex1                      (output.TexCoordAmbient)
#define o_fog                       (output.FogFactor)
#define o_tex_shadow0               (output.TexCoordShadow)
#define o_normal                    (output.Normal)
#define o_tangent                   (output.Tangent)
#define o_binormal                  (output.Binormal)
#define o_eye                       (output.EyeDirection)
#define o_reflect                   (output.Reflection)
#define o_aniso_tangent             (output.AnisoTangent)
#define o_normal_diff               (output.Normal)
#define o_normal_spec               (output.Binormal)
#define o_cornea_coord              (output.Tangent)
#define o_model_pos                 (output.WorldPosition)
// --------------------------------------------------------------------------------------------------------------------------
#endif /* COMFY_VS */
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#ifdef COMFY_PS
// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Fragment inputs:
#define a_color0                    (input.Color)
#define a_color1                    (input.ColorSecondary)
#define a_tex0                      (input.TexCoord)
#define a_tex_color0                (input.TexCoord)
#define a_tex_color1                (input.TexCoordAmbient)
#define a_tex_normal0               (input.TexCoord)
#define a_tex_specular              (input.TexCoord)
#define a_tex_lucency               (input.TexCoordAmbient)
#define a_fogcoord                  (float2(input.FogFactor, 0.0))
#define a_eye                       (input.EyeDirection)
#define a_normal                    (input.Normal)
#define a_tangent                   (input.Tangent)
#define a_binormal                  (input.Binormal)
#define a_reflect                   (input.Reflection)
#define a_aniso_tangent             (input.AnisoTangent)
#define a_normal_diff               (input.Normal)
#define a_normal_spec               (input.Binormal)
#define a_cornea_coord              (input.Tangent)
#define a_model_pos                 (input.WorldPosition)
#define fragment_position           (input.Position)
// --------------------------------------------------------------------------------------------------------------------------

// NOTE: Fragment outputs:
// --------------------------------------------------------------------------------------------------------------------------
#define o_color                     (outputColor)
// --------------------------------------------------------------------------------------------------------------------------
#endif /* COMFY_PS */
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: General
#define FLOAT1_ZERO                 ((float1)0)
#define FLOAT2_ZERO                 ((float2)0)
#define FLOAT3_ZERO                 ((float3)0)
#define FLOAT4_ZERO                 ((float4)0)
#define FLOAT1_ONE                  ((float1)1)
#define FLOAT2_ONE                  ((float2)1)
#define FLOAT3_ONE                  ((float3)1)
#define FLOAT4_ONE                  ((float4)1)
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Program environment:
#define mvp                         (transpose(CB_ModelViewProjection))
#define mv                          (transpose(CB_ModelView))
#define camera_mvi                  (CB_Scene.View)
#define model_mtx                   (CB_Model)
#define model_mtx_i                 (transpose(CB_Model))
#define model_mtx_it                (transpose(CB_Model))
// TODO:
#define nt_mtx                      (float3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
// TODO:
#define p_lit_luce                  (CB_Scene.LightColor)
// TODO: program.local[3] (CB_Material.Shininess) (?) 
#define p_shininess                 (float4(20.0, 0.0, 0.0, 0.0))
// TODO: program.env[7]
#define p_lit_spec                  (float4(24.190920, 17.838573, 4.641884, 0.0))
// TODO: program.env[9]
#define p_lit_back                  (float4(18.160395, 18.160395, 18.160395, 0.0))
#define p_morph_weight              (float4(CB_MorphWeight.xy, 0.0, 0.0))
// TODO:
#define p_texcol_coef               (float4(0.9, 0.0, 0.0, 0.0))
#define p_blend_color               (float4(1.0, 1.0, 1.0, 1.0))
#define p_offset_color              (float4(0.0, 0.0, 0.0, 1.0))
#define p_max_alpha                 (float4(1.0, 1.0, 1.0, 1.0))
#define p_fres_coef                 (CB_Material.FresnelCoefficient)
#define p_bump_depth                (CB_Material.BumpDepth)
// TODO: program.local[7]
#define p_texspc_coef               (float4(0.0, 0.0, 0.0, 0.0))
// TODO: program.local[8]
#define p_texspc_offset             (float4(0.0, 0.0, 0.0, 0.0))
// TODO: program.local[11]
#define p_ellipsoid_scale           (float4(0.02, 0.024, 0.009, 1.0))
// TODO: program.local[12]
#define p_tex_model_param           (float4(5.0, 5.0, 0.5, 0.5))
// TODO: program.local[13]
#define p_tex_offset                (float4(0.0, 0.0, 0.0, 0.0))
// TODO: program.local[14]
#define p_eb_radius                 (float4(1.0, 1.0, 1.0, 1.0))
// TODO: program.local[15]
#define p_eb_tex_model_param        (float4(2.5, 2.5, 0.5, 0.5))
// TODO: program.local[11]
#define p_ellipsoid_radius          (float4(2500.0, 1736.111084, 12345.680664, 0.000081))
// TODO: program.local[12]
#define p_fresnel                   (float4(0.966264, 0.033736, 0.0, 0.0))
// TODO: program.local[13]
#define p_refract1                  (float4(0.475624, 0.524376, 0.689655, 0.0))
// TODO: program.local[14]
#define p_refract2                  (float4(2.102500, -1.1025, 1.45, 0.0))
// TODO: program.local[15]
#define p_iris_radius               (float4(2500.0, 1736.111084, 40000.0, -1.0))
// TODO: program.local[16]
#define p_cornea_radius             (float4(2500.0, 1736.111084, 12345.680664, -1.0))
// TODO: program.local[17]
#define p_pupil_radius              (float4(10000.0, 6944.444336, 15624.998047, -1.0))
// TODO: program.local[18]
#define p_tex_scale                 (float4(10.000001, 8.333333, 61.314545, -0.004))
// TODO: program.env[25]
#define p_sss_param                 (float4(0.0, 0.0, 0.0, 0.0))
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
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: State fog:
#define state_fog_params            (CB_Scene.DepthFog.Parameters)
#define p_fog_color                 (CB_Scene.DepthFog.Color)
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
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
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
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
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
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
#define ABS(result, a)              result      =         ( abs(a) )
#define EX2(result, a)              result      =         ( exp2(a) )
#define EX2_SAT(result, a)          result      = saturate( exp2(a) )
#define POW(result, a, b)           result      =         ( pow( (a), (b) ) )
#define LRP(result, a, b, c)        result      =         ( lerp( (c), (b), (a) ) )
#define RCP(result, a)              result      =         ( rcp( (a) ) )
#define RSQ(result, a)              result      =         ( rsqrt( (a) ) )
#define DP2(result, a, b)           result.x    =         ( dot( (a).xy, (b).xy ) )
#define DP2_SAT(result, a, b)       result.x    = saturate( dot( (a).xy, (b).xy ) )
#define DP3(result, a, b)           result.x    =         ( dot( (a).xyz, (b).xyz ) )
#define DP3_SAT(result, a, b)       result.x    = saturate( dot( (a).xyz, (b).xyz ) )
#define DP4(result, a, b)           result.x    =         ( dot( (a).xyzw, (b).xyzw ) )
#define DP4_SAT(result, a, b)       result.x    = saturate( dot( (a).xyzw, (b).xyzw ) )
#define XPD(result, a, b)           result.xyz  =         ( cross( (a).xyz, (b).xyz ) )
#define NRM(result, a)              result.xyz  =         ( normalize( (a).xyz ) )
#define NRMH(result, a)             result.xyzw =         ( normalize( (a).xyzw ) )

// TODO: Might have to check b against a instead
#define SEQ(result, a, b)           if (a == b) result  = ( a )
#define SGE(result, a, b)           if (a >= b) result  = ( a )
#define SGT(result, a, b)           if (a >  b) result  = ( a )
#define SLE(result, a, b)           if (a <= b) result  = ( a )
#define SLT(result, a, b)           if (a <  b) result  = ( a )
#define SNE(result, a, b)           if (a != b) result  = ( a )

#define TEX2D_00(result, texCoord)  result = DiffuseTexture.Sample(DiffuseSampler, (texCoord).xy)
#define TEX2D_01(result, texCoord)  result = AmbientTexture.Sample(AmbientSampler, (texCoord).xy)
#define TEX2D_02(result, texCoord)  result = NormalTexture.Sample(NormalSampler, (texCoord).xy).xyzx
#define TEX2D_03(result, texCoord)  result = SpecularTexture.Sample(SpecularSampler, (texCoord).xy)
#define TEX2D_06(result, texCoord)  result = LucencyTexture.Sample(LucencySampler, (texCoord).xy)
// TODO: simple_reflect...
#define TEX2D_15(result, texCoord)  result = ScreenReflectionTexture.Sample(ScreenReflectionSampler, (texCoord).xy)
// TODO: subsurface scattering...
#define TEX2D_16(result, texCoord)  result = float4(0.0, 0.0, 0.0, 0.0)

#define TEXCUBE_05(result, texCoord) result = ReflectionCubeMap.Sample( ReflectionSampler, (texCoord).xyz )
#define TEXCUBE_09(result, texCoord) result = CharacterLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_10(result, texCoord) result = SunLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_11(result, texCoord) result = ReflectLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_12(result, texCoord) result = ShadowLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_13(result, texCoord) result = CharColorLightMap.Sample( LightMapSampler, (texCoord).xyz )

#define TXLCUBE_09(result, texCoord) result = CharacterLightMap.SampleLevel( LightMapSampler, (texCoord).xyz, (texCoord).w )

// --------------------------------------------------------------------------------------------------------------------------

// NOTE: Vertex shader snippets:
// --------------------------------------------------------------------------------------------------------------------------
#define VS_SET_MODEL_POSITION                                                                                               \
if (FLAGS_MORPH)                                                                                                            \
{                                                                                                                           \
    pos_m       = mad(a_position,   p_morph_weight.y, (a_morph_position * p_morph_weight.x));                               \
}                                                                                                                           \
else                                                                                                                        \
{                                                                                                                           \
    pos_m       = a_position;                                                                                               \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define VS_SET_MODEL_POSITION_NORMAL                                                                                        \
if (FLAGS_MORPH)                                                                                                            \
{                                                                                                                           \
    pos_m       = mad(a_position,   p_morph_weight.y, (a_morph_position * p_morph_weight.x));                               \
    normal_m    = mad(a_normal,     p_morph_weight.y, (a_morph_normal   * p_morph_weight.x));                               \
}                                                                                                                           \
else                                                                                                                        \
{                                                                                                                           \
    pos_m       = a_position;                                                                                               \
    normal_m    = a_normal;                                                                                                 \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define VS_SET_MODEL_POSITION_NORMAL_TANGENT                                                                                \
if (FLAGS_MORPH)                                                                                                            \
{                                                                                                                           \
    pos_m       = mad(a_position,   p_morph_weight.y, (a_morph_position * p_morph_weight.x));                               \
    normal_m    = mad(a_normal,     p_morph_weight.y, (a_morph_normal   * p_morph_weight.x));                               \
    tangent_m   = mad(a_tangent,    p_morph_weight.y, (a_morph_tangent  * p_morph_weight.x));                               \
}                                                                                                                           \
else                                                                                                                        \
{                                                                                                                           \
    pos_m       = a_position;                                                                                               \
    normal_m    = a_normal;                                                                                                 \
    tangent_m   = a_tangent;                                                                                                \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define VS_SET_OUTPUT_TEX_COORDS                                                                                            \
float4 __tex0, __tex1;                                                                                                      \
if (FLAGS_MORPH)                                                                                                            \
{                                                                                                                           \
    __tex0 = mad(a_tex0, p_morph_weight.y, (a_morph_texcoord  * p_morph_weight.x));                                         \
    __tex1 = mad(a_tex1, p_morph_weight.y, (a_morph_texcoord1 * p_morph_weight.x));                                         \
}                                                                                                                           \
else                                                                                                                        \
{                                                                                                                           \
    __tex0 = a_tex0;                                                                                                        \
    __tex1 = a_tex1;                                                                                                        \
}                                                                                                                           \
o_tex0 = float2(dot(state_matrix_texture0[0], __tex0), dot(state_matrix_texture0[1], __tex0));                              \
o_tex1 = float2(dot(state_matrix_texture1[0], __tex1), dot(state_matrix_texture1[1], __tex1));                              \
// --------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
#define VS_SET_DIFFUSE_IRRADIANCE                                                                                           \
diff.r = dot(mul(normal_w, irrad_r), normal_w);                                                                             \
diff.g = dot(mul(normal_w, irrad_g), normal_w);                                                                             \
diff.b = dot(mul(normal_w, irrad_b), normal_w);                                                                             \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define VS_A_COLOR_OR_MORPH ((FLAGS_MORPH) ? (mad(a_color, p_morph_weight.y, (a_morph_color * p_morph_weight.x))) : a_color)
// --------------------------------------------------------------------------------------------------------------------------

// NOTE: Pixel shader snippets:
// --------------------------------------------------------------------------------------------------------------------------
#define PS_ALPHA_TEST                                                                                                       \
if (FLAGS_ALPHA_TEST)                                                                                                       \
{                                                                                                                           \
    ClipAlphaThreshold(o_color.a);                                                                                          \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define PS_RET_O_COLOR(color)                                                                                               \
return float4( (color).rgb, 1.0 );                                                                                          \
// --------------------------------------------------------------------------------------------------------------------------

#endif /* DEBUGINTERFACE_HLSL */
