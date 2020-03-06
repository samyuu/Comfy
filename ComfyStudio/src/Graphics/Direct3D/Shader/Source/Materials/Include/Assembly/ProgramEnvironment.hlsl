#ifndef PROGRAMENVIRONMENT_HLSL
#define PROGRAMENVIRONMENT_HLSL

#include "../Defines/MathConstants.hlsl"

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Program environment:
#define mvp                         (transpose(CB_Object.ModelViewProjection))
#define mv                          (transpose(CB_Object.ModelView))
#define camera_mvi                  (CB_Scene.View)
#define model_mtx                   (transpose(CB_Object.Model))
#define model_mtx_i                 (transpose(CB_Object.Model))
#define model_mtx_it                (transpose(CB_Object.Model))

// TODO: { program.env[26 .. 28] }
#define nt_mtx                      (FLOAT3x3_IDENTITY)

// TODO: program.env[2]
#define p_blend_color               (float4(1.0, 1.0, 1.0, 1.0))
// TODO: program.env[3]
#define p_offset_color              (float4(0.0, 0.0, 0.0, 1.0))
// TODO: program.env[21]
#define p_max_alpha                 (float4(1.0, 1.0, 1.0, 1.0))

#define p_morph_weight              (float4(CB_Object.MorphWeight.xy, 0.0, 0.0))
#define p_fres_coef                 (CB_Object.Material.FresnelCoefficient)
#define p_bump_depth                (CB_Object.Material.BumpDepth)

#define p_sss_param                 (CB_Scene.SubsurfaceScatteringParameter)

// TODO: Should this be the same as p_fres_coef (?)
#define fres_coef                   (CB_Object.Material.FresnelCoefficient)
#define p_fb_isize                  (CB_Scene.TexelRenderResolution)
#define program_env_00              (float4(CB_Scene.TexelRenderResolution, 1.0, 1.0))

#define p_esm_k                     (CB_Scene.ShadowExponent)
#define program_env_12              (CB_Scene.ShadowAmbient)
#define program_env_13              (CB_Scene.OneMinusShadowAmbient)

// TODO:
#define program_env_17              (float4(0.06, 0.05, 0.88, 1.0))
// TODO:
#define program_env_19              (float4(0.0, 0.0, 0.0, 0.0))
#define program_env_24              (CB_Object.AmbientTextureType)

// TODO:
#define p_reflect_refract_uv_scale  (float4(0.1, 0.1, 0.1, 0.1))

// NOTE: Frensel effect approximation
#define reciprocal_one_minus_cos_pi_ten     (1.0 / (1.0 - cos(PI / 10.0)))
#define reciprocal_one_minus_cos_pi_four    (1.0 / (1.0 - cos(PI / 4.0)))

#define irrad_r                     (CB_Scene.IBLIrradianceRed)
#define irrad_g                     (CB_Scene.IBLIrradianceGreen)
#define irrad_b                     (CB_Scene.IBLIrradianceBlue)
#define lit_dir                     (CB_Scene.StageLight.Direction)
#define lit_dir_w                   (CB_Scene.StageLight.Direction)
#define lit_diff                    (CB_Scene.IBLStageColor)
#define lit_spec                    ((CB_Scene.StageLight.Specular.rgb * CB_Scene.IBLStageColor.rgb) * reciprocal_one_minus_cos_pi_ten)

#define p_shininess                 (float4(CB_Object.Material.Shininess.x, 0.0, 0.0, 0.0))
#define p_lit_luce                  ((CB_Scene.IBLCharaColor.yyyy) * reciprocal_one_minus_cos_pi_four)
#define p_lit_spec                  ((CB_Scene.CharaLight.Specular * CB_Scene.IBLCharaColor) * reciprocal_one_minus_cos_pi_ten)
#define p_lit_back                  ((CB_Scene.CharaLight.Specular * CB_Scene.IBLSunColor) * reciprocal_one_minus_cos_pi_ten)
#define p_lit_dir                   (CB_Scene.CharaLight.Direction)
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Program local:

// TODO: program.local[5]
#define p_texcol_coef               (float4(1.0, 1.0, 1.0, 0.0))
// TODO: program.local[6]
#define p_texcol_offset             (float4(0.0, 0.0, 0.0, 0.0))
// TODO: program.local[7]
#define p_texspc_coef               (float4(1.0, 1.0, 1.0, 0.015))
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
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: Debug:
#define dummy_color                 (float4(1.0, 0.0, 0.0, 1.0))
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: State fog:
#define state_fog_params            (CB_Scene.DepthFog.Parameters)
#define p_fog_color                 (CB_Scene.DepthFog.Color)
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: State light:
#define state_light0_ambient        (CB_Scene.CharaLight.Ambient)
#define state_light0_diffuse        (CB_Scene.CharaLight.Diffuse)
#define state_light0_specular       (CB_Scene.CharaLight.Specular)
#define state_light1_ambient        (CB_Scene.StageLight.Ambient)
#define state_light1_diffuse        (CB_Scene.StageLight.Diffuse)
#define state_light1_specular       (CB_Scene.StageLight.Specular)
#define state_lightprod1_ambient    (CB_Object.Material.Specular * CB_Scene.StageLight.Ambient)
#define state_lightprod1_diffuse    (CB_Object.Material.Diffuse * CB_Scene.StageLight.Diffuse)
#define state_lightprod1_specular   (CB_Object.Material.Specular * CB_Scene.StageLight.Specular)
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
// NOTE: State material:
#define state_matrix_texture0       (CB_Object.Material.DiffuseTextureTransform)
#define state_matrix_texture1       (CB_Object.Material.AmbientTextureTransform)
#define state_matrix_texture6       (CB_Scene.LightSpace)
#define state_matrix_texture7       (CB_Scene.LightSpace)
#define state_material_diffuse      (CB_Object.Material.Diffuse)
#define state_material_ambient      (CB_Object.Material.Ambient)
#define state_material_specular     (CB_Object.Material.Specular)
#define state_material_emission     (CB_Object.Material.Emission)
#define state_material_shininess    (CB_Object.Material.Shininess.y)
// --------------------------------------------------------------------------------------------------------------------------

#endif /* PROGRAMENVIRONMENT_HLSL */
