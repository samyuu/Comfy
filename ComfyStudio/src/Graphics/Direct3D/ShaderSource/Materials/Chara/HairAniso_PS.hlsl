#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE

    TEMP normal = FLOAT4_ZERO;
    TEMP rot_normal;
    TEMP lc, spec, diff, luce;
    TEMP col0;
    TEMP tmp;
    TEMP aniso_tangent, eye;
    TEMP aniso_coef;
    TEMP ndote;
    TEX2D_02(tmp, a_tex_normal0);
    MAD(tmp.xy, tmp.wy, 2.0, -1.0);
    MUL(tmp.zw, tmp.xy, tmp.xy);
    MUL(tmp.zw, tmp.zw, tmp.xy);
    MUL(tmp, tmp, float4(1.5, 1.5, 2.0, 2.0));
    ADD(tmp.xy, tmp.xy, tmp.zw);
    MUL(normal.xyz, a_tangent.xyz, tmp.x);
    MAD(normal.xyz, a_binormal.xyz, tmp.y, normal.xyz);
    ADD(normal.xyz, normal.xyz, a_normal.xyz);
    NRMH(normal, normal);
    NRM(eye, a_eye);
    DP3(rot_normal.x, nt_mtx[0], normal);
    DP3(rot_normal.y, nt_mtx[1], normal);
    DP3(rot_normal.z, nt_mtx[2], normal);
    MOV(lc, 1);
    DP3_SAT(tmp.x, normal, eye);
    MOV(ndote.x, tmp.x);
    MAD(tmp.x, tmp.x, -tmp.x, 1.0);
    POW(tmp.x, tmp.x, 8.0);
    MUL(tmp.x, tmp.x, a_color1.w);
    MUL(luce.xyz, p_lit_luce.xyz, tmp.x);
    // TEX2D_01(tmp, a_tex_lucency);
    TEX2D_06(tmp, a_tex_lucency);
    MUL(luce.xyz, luce.xyz, tmp.xyz);
    MUL(luce.xyz, luce.xyz, lc.z);
    MOV(rot_normal.w, 0);
    // TXL(diff, rot_normal, texture[9], CUBE);
    TXLCUBE_09(diff, rot_normal);
    MOV(rot_normal.w, 1);
    // TXL(tmp, rot_normal, texture[9], CUBE);
    TXLCUBE_09(tmp, rot_normal);
    LRP(diff, lc.y, diff, tmp);
    TEX2D_00(col0, a_tex_color0);
    MUL(col0.xyz, col0.xyz, p_texcol_coef.x);
    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    MOV(diff.xyz, diff.xyz);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    // NRMH(aniso_tangent.xyz, a_aniso_tangent);
    NRMH(aniso_tangent, a_aniso_tangent);
    DP3(aniso_tangent.w, aniso_tangent, normal);
    MAD(aniso_tangent.xyz, -aniso_tangent.w, normal.xyz, aniso_tangent.xyz);
    // NRMH(aniso_tangent.xyz, aniso_tangent);
    NRMH(aniso_tangent, aniso_tangent);
    DP3(tmp.x, aniso_tangent, p_lit_dir);
    DP3(tmp.y, aniso_tangent, eye);
    MOV(tmp.w, -tmp.x);
    MAD(tmp, tmp.xyxw, tmp.xyyy, float4(-1.01, -1.01, 0, 0));
    RSQ(aniso_coef.x, -tmp.x);
    RSQ(aniso_coef.y, -tmp.y);
    MUL(tmp.xy, -tmp.xy, aniso_coef.xy);
    // MAD_SAT(tmp.yz, tmp.x, tmp.y, -tmp.zzww);
    MAD_SAT(tmp.yz, tmp.x, tmp.y, -tmp.zw);
    POW(tmp.y, tmp.y, p_shininess.x);
    POW(tmp.z, tmp.z, p_shininess.x);
    MUL(tmp.x, tmp.x, tmp.x);
    DP3(aniso_coef.yz, normal, p_lit_dir);
    MAD_SAT(aniso_coef.yz, aniso_coef.xy, float2(0, 0.7), float2(0, 0.3));
    // MUL(tmp.yz, tmp, aniso_coef);
    MUL(tmp.yz, tmp.yz, aniso_coef.xy);
    MAD(aniso_coef, tmp, float4(0.25, 0.18, 0.05, 0), float4(0.75, 0, 0, 0));
    MUL(spec, p_lit_spec, aniso_coef.y);
    MUL(tmp, p_lit_back, aniso_coef.z);
    MAD(spec, spec, lc.z, tmp);
    ADD(spec.xyz, spec.xyz, p_texcol_coef.w);
    TEX2D_03(tmp, a_tex_specular);
    MUL(spec, spec, tmp);
    MUL(diff, diff, aniso_coef.x);
    MAD(diff, luce, 0.5, diff);
    MAD(diff.xyz, spec.xyz, state_material_specular.xyz, diff.xyz);
    MOV(diff.xyz, diff.xyz);
    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, diff.xyz);
    MAX(o_color.w, col0.w, p_max_alpha.w);
    
    PS_ALPHA_TEST;
    
#endif
    
    return outputColor;
}
