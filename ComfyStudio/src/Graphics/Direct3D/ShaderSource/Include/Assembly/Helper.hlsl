#ifndef HELPER_HLSL
#define HELPER_HLSL

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
#define VS_SET_OUTPUT_TEX_COORDS_NO_TRANSFORM                                                                               \
if (FLAGS_MORPH)                                                                                                            \
{                                                                                                                           \
    o_tex0 = mad(a_tex0, p_morph_weight.y, (a_morph_texcoord  * p_morph_weight.x)).xy;                                      \
    o_tex1 = mad(a_tex1, p_morph_weight.y, (a_morph_texcoord1 * p_morph_weight.x)).xy;                                      \
}                                                                                                                           \
else                                                                                                                        \
{                                                                                                                           \
    o_tex0 = a_tex0.xy;                                                                                                     \
    o_tex1 = a_tex1.xy;                                                                                                     \
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
#define PS_APPLY_SAMPLE_AMBIENT_TEX_COL                                                                                     \
TEX2D_01(_tmp0, a_tex_color1);                                                                                              \
                                                                                                                            \
if (program_env_24 == 0)                                                                                                    \
{                                                                                                                           \
    LRP(tex_col.xyz, _tmp0.w, _tmp0.xyz, tex_col.xyz);                                                                      \
}                                                                                                                           \
else if (program_env_24 == 1)                                                                                               \
{                                                                                                                           \
    MUL(tex_col, _tmp0, tex_col);                                                                                           \
}                                                                                                                           \
else if (program_env_24 == 2)                                                                                               \
{                                                                                                                           \
    ADD(tex_col.xyz, _tmp0.xyz, tex_col.xyz);                                                                               \
    MUL(tex_col.w, _tmp0.w, tex_col.w);                                                                                     \
}                                                                                                                           \
else if (program_env_24 == 3)                                                                                               \
{                                                                                                                           \
    ADD(_tmp2.w, _tmp0.w, 0.004);                                                                                           \
    RCP(_tmp2.w, _tmp2.w);                                                                                                  \
    MUL(_tmp2.xyz, _tmp0.xyz, _tmp2.w);                                                                                     \
    MUL(tex_col.xyz, _tmp2.xyz, tex_col.xyz);                                                                               \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------------
#define PS_ALPHA_TEST                                                                                                       \
if (FLAGS_ALPHA_TEST)                                                                                                       \
{                                                                                                                           \
    ClipAlphaThreshold(o_color.a);                                                                                          \
}                                                                                                                           \
// --------------------------------------------------------------------------------------------------------------------------

#endif /* HELPER_HLSL */
