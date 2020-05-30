#include "../../Include/InputLayout.hlsl"
#include "../../Include/ConstantInputs.hlsl"
#include "../../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP _tmp0 = FLOAT4_ZERO, _tmp1, _tmp2;
    TEMP dev_pos, lc;
    TEMP eye;
    TEMP diff;
    TEMP spec1;
    TEMP spec2;
    TEMP spec3;
    TEMP col0;
    TEMP col1;
    TEMP col2;
    TEMP len, trsmit, trsmit_coef;
    TEMP cosine, coef, tmp, tmp2;
    TEMP pos_cornea1 = FLOAT4_ZERO;
    TEMP pos_cornea2 = FLOAT4_ZERO;
    TEMP pos_iris = FLOAT4_ZERO;
    TEMP pos_pupil = FLOAT4_ZERO;
    TEMP nrm_cornea1;
    TEMP nrm_cornea2;
    TEMP nrm_iris;
    TEMP nrm_pupil;
    TEMP ray;
    TEMP rot_ray;
    TEMP _ftmp0;
    
    MOV(o_color, float4(1, 0, 0, 1));
    MOV(pos_cornea1, a_model_pos);
    MUL(tmp.xy, pos_cornea1.xy, p_ellipsoid_radius.xy);
    DP2(tmp.w, tmp, pos_cornea1);
    MAD(tmp.w, -tmp.w, p_ellipsoid_radius.w, p_ellipsoid_radius.w);

    //SUBC(tmp.z, tmp.w, 1.0e-6);
    SUB(tmp.z, tmp.w, 1.0e-6);

    RSQ(tmp.x, tmp.w);

    //MUL(pos_cornea1.z (GT.z), tmp.x, tmp.w);
    if (tmp.z > 0.0)
	    MUL(pos_cornea1.z, tmp.x, tmp.w);

    MUL(tmp, 2, pos_cornea1);
    MUL(tmp, tmp, p_ellipsoid_radius);
    NRM(tmp, tmp);
    DP3(nrm_cornea1.x, model_mtx[0], tmp);
    DP3(nrm_cornea1.y, model_mtx[1], tmp);
    DP3(nrm_cornea1.z, model_mtx[2], tmp);
    NRM(eye, a_eye);
    
#if 0
    if (FLAGS_SELF_SHADOW)
    {
        float4 org_normal = a_normal_diff;
        PS_SAMPLE_SELF_SHADOW_MAP;
    }
    else
    {
        MOV(lc, 1);
    }
#else
    MOV(lc, 1);
#endif
    
    TEX2D_03(col2, a_tex_color1);
    MUL(col2.xyz, col2.w, 0.9);
    DP2(tmp.w, a_cornea_coord, a_cornea_coord);

    //SUBC(tmp.w, tmp.w, 0.96);
    SUB(tmp.w, tmp.w, 0.96);

    if (tmp.w > 0.0)
    {
	    NRM(nrm_cornea1, a_normal_diff);
	    DP3(rot_ray.x, nt_mtx[0], nrm_cornea1);
	    DP3(rot_ray.y, nt_mtx[1], nrm_cornea1);
	    DP3(rot_ray.z, nt_mtx[2], nrm_cornea1);
	    MOV(rot_ray.w, 0);
	    
        TXLCUBE_09(diff, rot_ray);
	    MOV(rot_ray.w, 1);
	    TXLCUBE_09(diff, rot_ray);
	    
        LRP(diff.xyz, lc.y, diff.xyz, tmp.xyz);
	    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
	    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
	    MOV(diff.xyz, diff.xyz);
	    MUL(col2.xyz, col2.xyz, diff.xyz);
	    MUL(dev_pos, fragment_position, program_env_00);
	    TEX2D_16(tmp, dev_pos);
	    LRP(diff.xyz, p_sss_param.x, tmp.xyz, diff.xyz);
	    TEX2D_00(col0, a_tex_color0);
	    MUL(diff.xyz, diff.xyz, col0.xyz);
	    DP3(ray.x, eye, a_normal_spec);
	    MUL(ray.x, ray.x, 2.0);
	    MAD(ray.xyz, ray.x, a_normal_spec.xyz, -eye.xyz);
	    DP3(rot_ray.x, nt_mtx[0], ray);
	    DP3(rot_ray.y, nt_mtx[1], ray);
	    DP3(rot_ray.z, nt_mtx[2], ray);
	    TEXCUBE_10(tmp, rot_ray);
	    TEXCUBE_12(tmp2, rot_ray);
	    LRP(tmp.xyz, lc.z, tmp.xyz, tmp2.xyz);
	    MUL(tmp.xyz, tmp.xyz, state_light0_specular.xyz);
	    LRP(tmp.xyz, 0.94, diff.xyz, tmp.xyz);
	    ADD(tmp.xyz, tmp.xyz, col2.xyz);
	    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, tmp.xyz);

	    return o_color;
    }

    DP3_SAT(cosine.x, eye, nrm_cornea1);
    MUL(cosine.y, cosine.x, cosine.x);
    MAD(cosine.y, cosine.y, p_refract1.x, p_refract1.y);

    //SUBC(_ftmp0.x, cosine.y, 1.0e-6);
    SUB(_ftmp0.x, cosine.y, 1.0e-6);

    RSQ(_ftmp0.w, cosine.y);
    MUL(cosine.y, _ftmp0.w, cosine.y);

    // MOV(cosine.y (LT.x), 1);
    if (_ftmp0.x < 0.0)
	    MOV(cosine.y, 1);

    SUB_SAT(spec1.w, 1, cosine.x);
    POW(spec1.w, spec1.w, 5.0);
    MAD(spec1.w, spec1.w, p_fresnel.x, p_fresnel.y);
    MUL(tmp.w, 2, state_material_specular.w);
    MIN(spec1.w, spec1.w, tmp.w);
    DP3(ray.x, eye, nrm_cornea1);
    MUL(ray.x, ray.x, 2.0);
    MAD(ray.xyz, ray.x, nrm_cornea1.xyz, -eye.xyz);
    DP3(rot_ray.x, nt_mtx[0], ray);
    DP3(rot_ray.y, nt_mtx[1], ray);
    DP3(rot_ray.z, nt_mtx[2], ray);
    TEXCUBE_05(tmp, rot_ray);
    MUL(tmp.xyz, tmp.xyz, tmp.xyz);
    MAD(tmp.xyz, tmp.xyz, 7.0, float3(-0.5, -0.5, -0.5));
    MAX(tmp.xyz, tmp.xyz, 0);
    MUL(spec1.xyz, tmp.xyz, state_light0_specular.xyz);
    MAD(ray.x, cosine.x, p_refract1.z, -cosine.y);
    MUL(ray, nrm_cornea1, ray.x);
    MAD(ray, eye, -p_refract1.z, ray);
    DP3(tmp.x, model_mtx_i[0], ray);
    DP3(tmp.y, model_mtx_i[1], ray);
    DP3(tmp.z, model_mtx_i[2], ray);
    MOV(ray.xyz, tmp.xyz);
    MOV(ray.w, 0);
    MOV(len, 0);
    MUL(_ftmp0.xyz, ray.xyz, p_iris_radius.xyz);
    DP3(coef.x, _ftmp0, ray);
    DP3(coef.y, _ftmp0, pos_cornea1);
    MUL(coef.y, coef.y, 2);
    MUL(_ftmp0, pos_cornea1, p_iris_radius);
    DP4(coef.z, _ftmp0, pos_cornea1);
    MUL(_ftmp0.xy, coef.xy, coef.zy);
    MAD(_ftmp0.z, _ftmp0.x, -4, _ftmp0.y);

    //SUBC(_ftmp0.w, _ftmp0.z, 1.0e-6);
    SUB(_ftmp0.w, _ftmp0.z, 1.0e-6);

    RSQ(_ftmp0.w, _ftmp0.z);
    MUL(_ftmp0.w, _ftmp0.z, _ftmp0.w);
    MAD(_ftmp0.zw, float2(+1, -1), _ftmp0.w, -coef.y);
    RCP(_ftmp0.x, coef.x);
    MUL(_ftmp0.x, _ftmp0.x, 0.5);
    MUL(tmp.xy, _ftmp0.x, _ftmp0.zw);
    MOV(o_color, float4(0, 0, 1, 1));
    //RET((LT.w));
    if (_ftmp0.w < 0.0)
	    return o_color;

    //MADC(pos_iris, ray, tmp.x, pos_cornea1);
    MAD(pos_iris, ray, tmp.x, pos_cornea1);

    MOV(o_color, float4(0, 1, 0, 1));
    //RET((GT.z));
    if (pos_iris.z > 0.0)
	    return o_color;

    ABS(len.x, tmp.x);
    MAD(tmp.xy, pos_iris.xy, p_tex_scale.xy, float2(0.5, 0.5));
    MAD(tmp.xy, tmp.xy, float2(-1, 1), float2(1, 0));
    TEX2D_00(col0, tmp);
    MOV(col1, col0);
    SUB(pos_cornea1.z, pos_cornea1.z, p_tex_scale.w);
    MUL(_ftmp0.xyz, ray.xyz, p_pupil_radius.xyz);
    DP3(coef.x, _ftmp0, ray);
    DP3(coef.y, _ftmp0, pos_cornea1);
    MUL(coef.y, coef.y, 2);
    MUL(_ftmp0, pos_cornea1, p_pupil_radius);
    DP4(coef.z, _ftmp0, pos_cornea1);
    MUL(_ftmp0.xy, coef.xy, coef.zy);
    MAD(_ftmp0.z, _ftmp0.x, -4, _ftmp0.y);
    
    //SUBC(_ftmp0.w, _ftmp0.z, 1.0e-6);
    SUB(_ftmp0.w, _ftmp0.z, 1.0e-6);

    RSQ(_ftmp0.w, _ftmp0.z);
    MUL(_ftmp0.w, _ftmp0.z, _ftmp0.w);
    MAD(_ftmp0.zw, float2(+1, -1), _ftmp0.w, -coef.yy);
    RCP(_ftmp0.x, coef.x);
    MUL(_ftmp0.x, _ftmp0.x, 0.5);
    MUL(tmp.xy, _ftmp0.x, _ftmp0.zw);

    //MOV(tmp.y (LT.w), 999999);
    if (_ftmp0.w < 0.0)
	    MOV(tmp.y, 999999);

    //SUBC(tmp.w, tmp.y, len.x);
    SUB(tmp.w, tmp.y, len.x);

    MOV(spec3, 0);

    //IF(LT.w);
    if (tmp.w < 0.0)
    {
	    MOV(len.x, tmp.y);
	    MAD(pos_pupil, ray, tmp.y, pos_cornea1);
	    MUL(nrm_pupil, pos_pupil, p_pupil_radius);
	    MUL(nrm_pupil, nrm_pupil, 2);
	    NRM(nrm_pupil, nrm_pupil);
	    MOV(tmp, -ray);
	    DP3(tmp2.x, tmp, nrm_pupil);
	    MUL(tmp2.x, tmp2.x, 2.0);
	    MAD(tmp2.xyz, tmp2.x, nrm_pupil.xyz, -tmp.xyz);
	    DP3(tmp.x, model_mtx[0], tmp2);
	    DP3(tmp.y, model_mtx[1], tmp2);
	    DP3(tmp.z, model_mtx[2], tmp2);
	    DP3(rot_ray.x, nt_mtx[0], tmp);
	    DP3(rot_ray.y, nt_mtx[1], tmp);
	    DP3(rot_ray.z, nt_mtx[2], tmp);
	    TEXCUBE_10(tmp2, rot_ray);
	    MUL(spec3.xyz, tmp2.xyz, col1.xyz);
	    MUL(spec3.xyz, spec3.xyz, 0.10);
	    MUL(spec3.xyz, spec3.xyz, state_light0_specular.xyz);
	    MUL(col0, col0, 0.75);
    }

    MUL(nrm_iris, pos_iris, p_iris_radius);
    MUL(nrm_iris, nrm_iris, -2);
    NRM(nrm_iris, nrm_iris);
    DP3(spec2.w, col1, col1);
    MAD_SAT(spec2.w, spec2.w, 0.20, -0.02);
    DP3(tmp.x, model_mtx[0], nrm_iris);
    DP3(tmp.y, model_mtx[1], nrm_iris);
    DP3(tmp.z, model_mtx[2], nrm_iris);
    MOV(tmp2, tmp);
    DP3(rot_ray.x, nt_mtx[0], tmp2);
    DP3(rot_ray.y, nt_mtx[1], tmp2);
    DP3(rot_ray.z, nt_mtx[2], tmp2);
    MOV(rot_ray.w, 0);
    TXLCUBE_09(diff, rot_ray);
    MOV(rot_ray.w, 1);
    TXLCUBE_09(tmp, rot_ray);
    LRP(diff.xyz, lc.y, diff.xyz, tmp.xyz);
    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    MOV(diff.xyz, diff.xyz);
    MUL(col2.xyz, col2.xyz, diff.xyz);
    MUL(diff.xyz, diff.xyz, 0.95);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    MAD(tmp.xyz, -2, state_material_diffuse.xyz, float3(2, 2, 2));
    MUL(trsmit_coef.xyz, tmp.xyz, p_tex_scale.z);
    MUL(_ftmp0.xyz, -len.x, trsmit_coef.xyz);
    EX2(trsmit.x, _ftmp0.x);
    EX2(trsmit.y, _ftmp0.y);
    EX2(trsmit.z, _ftmp0.z);
    MUL(col0.xyz, diff.xyz, trsmit.xyz);
    LRP(col0.xyz, spec1.w, spec1.xyz, col0.xyz);
    ADD(col0.xyz, col0.xyz, col2.xyz);
    MOV(tmp, -ray);
    DP3(ray.x, tmp, nrm_iris);
    MUL(ray.x, ray.x, 2.0);
    MAD(ray.xyz, ray.x, nrm_iris.xyz, -tmp.xyz);
    MUL(_ftmp0.xyz, ray.xyz, p_cornea_radius.xyz);
    DP3(coef.x, _ftmp0, ray);
    DP3(coef.y, _ftmp0, pos_iris);
    MUL(coef.y, coef.y, 2);
    MUL(_ftmp0, pos_iris, p_cornea_radius);
    DP4(coef.z, _ftmp0, pos_iris);
    MUL(_ftmp0.xy, coef.xy, coef.zy);
    MAD(_ftmp0.z, _ftmp0.x, -4, _ftmp0.y);

    //SUBC(_ftmp0.w, _ftmp0.z, 1.0e-6);
    SUB(_ftmp0.w, _ftmp0.z, 1.0e-6);

    RSQ(_ftmp0.w, _ftmp0.z);
    MUL(_ftmp0.w, _ftmp0.z, _ftmp0.w);
    MAD(_ftmp0.zw, float2(+1, -1), _ftmp0.w, -coef.yy);
    RCP(_ftmp0.x, coef.x);
    MUL(_ftmp0.x, _ftmp0.x, 0.5);
    MUL(tmp.xy, _ftmp0.x, _ftmp0.zw);
    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, col0.xyz);

    if (_ftmp0.w < 0.0)
	    return o_color;

    //MADC(pos_cornea2, ray, tmp.x, pos_iris);
    MAD(pos_cornea2, ray, tmp.x, pos_iris);
    //RET((LT.z));
    if (pos_cornea2.z < 0.0)
	    return o_color;

    ABS(len.y, tmp.x);
    MUL(nrm_cornea2, pos_cornea2, p_cornea_radius);
    MUL(nrm_cornea2, nrm_cornea2, 2);
    NRM(nrm_cornea2, nrm_cornea2);
    MOV(ray, -ray);
    MOV(nrm_cornea2, -nrm_cornea2);
    DP3_SAT(cosine.x, ray, nrm_cornea2);
    MUL(cosine.y, cosine.x, cosine.x);
    MAD(cosine.y, cosine.y, p_refract2.x, p_refract2.y);

    //SUBC(tmp.x, cosine.y, 1.0e-6);
    SUB(tmp.x, cosine.y, 1.0e-6);

    MUL(tmp.xyz, col0.xyz, 0.85);
    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, tmp.xyz);

    //RET((LT.x));
    if (tmp.x < 0.0)
	    return o_color;

    RSQ(tmp.w, cosine.y);
    MUL(cosine.y, tmp.w, cosine.y);
    MAD(tmp.x, cosine.x, p_refract2.z, -cosine.y);
    MUL(tmp, nrm_cornea2, tmp.x);
    MAD(tmp, ray, -p_refract2.z, tmp);
    DP3(ray.x, model_mtx[0], tmp);
    DP3(ray.y, model_mtx[1], tmp);
    DP3(ray.z, model_mtx[2], tmp);
    DP3(rot_ray.x, nt_mtx[0], ray);
    DP3(rot_ray.y, nt_mtx[1], ray);
    DP3(rot_ray.z, nt_mtx[2], ray);
    TEXCUBE_10(tmp, rot_ray);
    TEXCUBE_12(tmp2, rot_ray);
    LRP(tmp.xyz, lc.z, tmp.xyz, tmp2.xyz);
    MUL(spec2.xyz, tmp.xyz, state_light0_specular.xyz);
    MUL(spec2.xyz, spec2.xyz, col1.xyz);
    MUL(_ftmp0.xyz, -len.y, trsmit_coef.xyz);
    EX2(tmp.x, _ftmp0.x);
    EX2(tmp.y, _ftmp0.y);
    EX2(tmp.z, _ftmp0.z);
    MUL(spec2.xyz, spec2.xyz, tmp.xyz);
    MAD(diff.xyz, spec2.w, spec2.xyz, diff.xyz);
    MUL(diff.xyz, diff.xyz, trsmit.xyz);
    LRP(tmp.xyz, spec1.w, spec1.xyz, diff.xyz);
    ADD(tmp.xyz, tmp.xyz, spec3.xyz);
    ADD(tmp.xyz, tmp.xyz, col2.xyz);
    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, tmp.xyz);
       
#endif
    
    return outputColor;
}
