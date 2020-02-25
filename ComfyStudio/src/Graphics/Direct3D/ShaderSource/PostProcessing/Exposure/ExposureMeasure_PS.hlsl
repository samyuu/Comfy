struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer ExposureConstantData : register(b9)
{
    float4 CB_SpotWeight;
    float4 CB_SpotCoefficients[32];
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float4> ScreenTexture : register(t0);
Texture2D<float4> ScreenTextureAlt : register(t1);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    #define p_center_coef float4(0.8, 1.0, 1.2, 0.0)
    #define p_spot_weight CB_SpotWeight
    #define p_spot_coef CB_SpotCoefficients
    #define TEX2D_00(texCoord) ScreenTexture.Sample(LinearTextureSampler, (texCoord).xy)
    #define TEX2D_01(texCoord) ScreenTextureAlt.Sample(LinearTextureSampler, (texCoord).xy)
    
    float4 sum, col, center, spot;
    sum = float4(0.0, 0.0, 1.0e-9, 0.0);
    col = TEX2D_00(float2(0.125, 0.125));
    sum.w = mad(col.w, p_center_coef.x, sum.w);
    sum.z = sum.z + p_center_coef.x;
    col = TEX2D_00(float2(0.375, 0.125));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.625, 0.125));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.875, 0.125));
    sum.w = mad(col.w, p_center_coef.x, sum.w);
    sum.z = sum.z + p_center_coef.x;
    col = TEX2D_00(float2(0.125, 0.375));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.375, 0.375));
    sum.w = mad(col.w, p_center_coef.z, sum.w);
    sum.z = sum.z + p_center_coef.z;
    col = TEX2D_00(float2(0.625, 0.375));
    sum.w = mad(col.w, p_center_coef.z, sum.w);
    sum.z = sum.z + p_center_coef.z;
    col = TEX2D_00(float2(0.875, 0.375));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.125, 0.625));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.375, 0.625));
    sum.w = mad(col.w, p_center_coef.z, sum.w);
    sum.z = sum.z + p_center_coef.z;
    col = TEX2D_00(float2(0.625, 0.625));
    sum.w = mad(col.w, p_center_coef.z, sum.w);
    sum.z = sum.z + p_center_coef.z;
    col = TEX2D_00(float2(0.875, 0.625));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.125, 0.875));
    sum.w = mad(col.w, p_center_coef.x, sum.w);
    sum.z = sum.z + p_center_coef.x;
    col = TEX2D_00(float2(0.375, 0.875));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.625, 0.875));
    sum.w = mad(col.w, p_center_coef.y, sum.w);
    sum.z = sum.z + p_center_coef.y;
    col = TEX2D_00(float2(0.875, 0.875));
    sum.w = mad(col.w, p_center_coef.x, sum.w);
    sum.z = sum.z + p_center_coef.x;
    sum.z = rcp(sum.z);
    center.w = sum.w * sum.z;
    
    sum = float4(0.0, 0.0, 1.0e-9, 0.0);
    col = TEX2D_01(p_spot_coef[0]);
    sum.w = mad(col.w, p_spot_coef[0].w, sum.w);
    sum.z = sum.z + p_spot_coef[0].w;
    col = TEX2D_01(p_spot_coef[1]);
    sum.w = mad(col.w, p_spot_coef[1].w, sum.w);
    sum.z = sum.z + p_spot_coef[1].w;
    col = TEX2D_01(p_spot_coef[2]);
    sum.w = mad(col.w, p_spot_coef[2].w, sum.w);
    sum.z = sum.z + p_spot_coef[2].w;
    col = TEX2D_01(p_spot_coef[3]);
    sum.w = mad(col.w, p_spot_coef[3].w, sum.w);
    sum.z = sum.z + p_spot_coef[3].w;
    col = TEX2D_01(p_spot_coef[4]);
    sum.w = mad(col.w, p_spot_coef[4].w, sum.w);
    sum.z = sum.z + p_spot_coef[4].w;
    col = TEX2D_01(p_spot_coef[5]);
    sum.w = mad(col.w, p_spot_coef[5].w, sum.w);
    sum.z = sum.z + p_spot_coef[5].w;
    col = TEX2D_01(p_spot_coef[6]);
    sum.w = mad(col.w, p_spot_coef[6].w, sum.w);
    sum.z = sum.z + p_spot_coef[6].w;
    col = TEX2D_01(p_spot_coef[7]);
    sum.w = mad(col.w, p_spot_coef[7].w, sum.w);
    sum.z = sum.z + p_spot_coef[7].w;
    sum.z = sum.z * 1.1;
    sum.z = rcp(sum.z);
    spot.x = sum.w * sum.z;
    
    sum = float4(0.0, 0.0, 1.0e-9, 0.0);
    col = TEX2D_01(p_spot_coef[8]);
    sum.w = mad(col.w, p_spot_coef[8].w, sum.w);
    sum.z = sum.z + p_spot_coef[8].w;
    col = TEX2D_01(p_spot_coef[9]);
    sum.w = mad(col.w, p_spot_coef[9].w, sum.w);
    sum.z = sum.z + p_spot_coef[9].w;
    col = TEX2D_01(p_spot_coef[10]);
    sum.w = mad(col.w, p_spot_coef[10].w, sum.w);
    sum.z = sum.z + p_spot_coef[10].w;
    col = TEX2D_01(p_spot_coef[11]);
    sum.w = mad(col.w, p_spot_coef[11].w, sum.w);
    sum.z = sum.z + p_spot_coef[11].w;
    col = TEX2D_01(p_spot_coef[12]);
    sum.w = mad(col.w, p_spot_coef[12].w, sum.w);
    sum.z = sum.z + p_spot_coef[12].w;
    col = TEX2D_01(p_spot_coef[13]);
    sum.w = mad(col.w, p_spot_coef[13].w, sum.w);
    sum.z = sum.z + p_spot_coef[13].w;
    col = TEX2D_01(p_spot_coef[14]);
    sum.w = mad(col.w, p_spot_coef[14].w, sum.w);
    sum.z = sum.z + p_spot_coef[14].w;
    col = TEX2D_01(p_spot_coef[15]);
    sum.w = mad(col.w, p_spot_coef[15].w, sum.w);
    sum.z = sum.z + p_spot_coef[15].w;
    sum.z = sum.z * 1.1;
    sum.z = rcp(sum.z);
    spot.y = sum.w * sum.z;
    
    sum = float4(0.0, 0.0, 1.0e-9, 0.0);
    col = TEX2D_01(p_spot_coef[16]);
    sum.w = mad(col.w, p_spot_coef[16].w, sum.w);
    sum.z = sum.z + p_spot_coef[16].w;
    col = TEX2D_01(p_spot_coef[17]);
    sum.w = mad(col.w, p_spot_coef[17].w, sum.w);
    sum.z = sum.z + p_spot_coef[17].w;
    col = TEX2D_01(p_spot_coef[18]);
    sum.w = mad(col.w, p_spot_coef[18].w, sum.w);
    sum.z = sum.z + p_spot_coef[18].w;
    col = TEX2D_01(p_spot_coef[19]);
    sum.w = mad(col.w, p_spot_coef[19].w, sum.w);
    sum.z = sum.z + p_spot_coef[19].w;
    col = TEX2D_01(p_spot_coef[20]);
    sum.w = mad(col.w, p_spot_coef[20].w, sum.w);
    sum.z = sum.z + p_spot_coef[20].w;
    col = TEX2D_01(p_spot_coef[21]);
    sum.w = mad(col.w, p_spot_coef[21].w, sum.w);
    sum.z = sum.z + p_spot_coef[21].w;
    col = TEX2D_01(p_spot_coef[22]);
    sum.w = mad(col.w, p_spot_coef[22].w, sum.w);
    sum.z = sum.z + p_spot_coef[22].w;
    col = TEX2D_01(p_spot_coef[23]);
    sum.w = mad(col.w, p_spot_coef[23].w, sum.w);
    sum.z = sum.z + p_spot_coef[23].w;
    sum.z = sum.z * 1.1;
    sum.z = rcp(sum.z);
    spot.z = sum.w * sum.z;
    
    sum = float4(0.0, 0.0, 1.0e-9, 0.0);
    col = TEX2D_01(p_spot_coef[24]);
    sum.w = mad(col.w, p_spot_coef[24].w, sum.w);
    sum.z = sum.z + p_spot_coef[24].w;
    col = TEX2D_01(p_spot_coef[25]);
    sum.w = mad(col.w, p_spot_coef[25].w, sum.w);
    sum.z = sum.z + p_spot_coef[25].w;
    col = TEX2D_01(p_spot_coef[26]);
    sum.w = mad(col.w, p_spot_coef[26].w, sum.w);
    sum.z = sum.z + p_spot_coef[26].w;
    col = TEX2D_01(p_spot_coef[27]);
    sum.w = mad(col.w, p_spot_coef[27].w, sum.w);
    sum.z = sum.z + p_spot_coef[27].w;
    col = TEX2D_01(p_spot_coef[28]);
    sum.w = mad(col.w, p_spot_coef[28].w, sum.w);
    sum.z = sum.z + p_spot_coef[28].w;
    col = TEX2D_01(p_spot_coef[29]);
    sum.w = mad(col.w, p_spot_coef[29].w, sum.w);
    sum.z = sum.z + p_spot_coef[29].w;
    col = TEX2D_01(p_spot_coef[30]);
    sum.w = mad(col.w, p_spot_coef[30].w, sum.w);
    sum.z = sum.z + p_spot_coef[30].w;
    col = TEX2D_01(p_spot_coef[31]);
    sum.w = mad(col.w, p_spot_coef[31].w, sum.w);
    sum.z = sum.z + p_spot_coef[31].w;
    sum.z = sum.z * 1.1;
    sum.z = rcp(sum.z);
    spot.w = sum.w * sum.z;
    
    sum.w = center.w;
    sum.z = 1;
    sum.w = mad(spot.x, p_spot_weight.x, sum.w);
    sum.z = sum.z + p_spot_weight.x;
    sum.w = mad(spot.y, p_spot_weight.y, sum.w);
    sum.z = sum.z + p_spot_weight.y;
    sum.w = mad(spot.z, p_spot_weight.z, sum.w);
    sum.z = sum.z + p_spot_weight.z;
    sum.w = mad(spot.w, p_spot_weight.w, sum.w);
    sum.z = sum.z + p_spot_weight.w;
    sum.z = rcp(sum.z);
    
    return sum.w * sum.z;
}
