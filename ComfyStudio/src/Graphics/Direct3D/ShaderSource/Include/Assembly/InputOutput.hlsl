#ifndef INPUTOUTPUT_HLSL
#define INPUTOUTPUT_HLSL

// --------------------------------------------------------------------------------------------------------------------------
#ifdef COMFY_VS
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
#endif /* COMFY_VS */
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#ifdef COMFY_PS
// NOTE: Fragment inputs:
#define a_color0                    (input.Color)
#define a_color1                    (input.ColorSecondary)
#define a_tex0                      (input.TexCoord)
#define a_tex_color0                (input.TexCoord)
#define a_tex_color1                (input.TexCoordAmbient)
#define a_tex_normal0               (input.TexCoord)
#define a_tex_specular              (input.TexCoord)
#define a_tex_lucency               (input.TexCoordAmbient)
#define a_tex_shadow0               (input.TexCoordShadow)
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

// NOTE: Fragment outputs:
#define o_color                     (outputColor)
#endif /* COMFY_PS */
// --------------------------------------------------------------------------------------------------------------------------

#endif /* INPUTOUTPUT_HLSL */
