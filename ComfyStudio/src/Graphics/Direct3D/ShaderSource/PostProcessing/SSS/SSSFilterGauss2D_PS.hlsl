struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer SSSFilterConstantData : register(b5)
{
    float4 CB_TextureSize;
    float4 CB_Coefficients[36];
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D SubsurfaceScatteringTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    const float4 p_color = float4(1.0, 0.96, 1.0, 0.0);
    const float4 p_tex_ofs = float4(5.0, 0.0, 1.0, 1.0);

    float4 col0, col1, col2, col3;
    
    float2 step = 0;
    float2 step_t = 0;
    
    step.x = CB_TextureSize.x * p_tex_ofs.z;
    step_t.y = CB_TextureSize.y * p_tex_ofs.w;
    
    float3 sum = 0.00001;
    float3 sum_coef = 0.00001;
    
    col0 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord);
    float4 o_color = col0;
    
    if ((col0.w - 0.5) < 0.0)
        return o_color;
    
    if ((col0.w - 0.5) < 0.0)
        return o_color;
        
    int index = 0;
    
    sum = mad(col0.xyz, CB_Coefficients[index].xyz, sum);
    sum_coef = CB_Coefficients[index].xyz + sum_coef;
    
    index++;
    
    float2 stex0 = input.TexCoord;
    float2 stex1 = input.TexCoord;
    
    float3 _tmp0;
    
    [unroll]
    for (int i = 0; i < p_tex_ofs.x; i++)
    {
        stex0 = stex0 + step;
        stex1 = stex1 - step;
    
        col0 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex0);
        col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex1);
    
        _tmp0.xyz = CB_Coefficients[index].xyz * col0.w;
        sum = mad(col0.xyz, _tmp0, sum);
        sum_coef = _tmp0.xyz + sum_coef;
        _tmp0.xyz = CB_Coefficients[index].xyz * col1.w;
        sum = mad(col1.xyz, _tmp0, sum);
        sum_coef = _tmp0.xyz + sum_coef;
    
        index++;
    }            
    
    float2 ttex0 = input.TexCoord;
    float2 ttex2 = input.TexCoord;
    
    [unroll]
    for (int j = 0; j < p_tex_ofs.x; j++)
    {
        float2 stex2, stex3;
        
        ttex0 = +step_t + ttex0;
        ttex2 = -step_t + ttex2;
        stex0 = ttex0;
        stex1 = ttex0;
        stex2 = ttex2;
        stex3 = ttex2;
        
        col0 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex0);
        col2 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex2);
        
        _tmp0.xyz = CB_Coefficients[index].xyz * col0.w;
        sum = mad(col0.xyz, _tmp0, sum);
        sum_coef = _tmp0.xyz + sum_coef;
        _tmp0.xyz = CB_Coefficients[index].xyz * col2.w;
        sum = mad(col2.xyz, _tmp0, sum);
        sum_coef = _tmp0.xyz + sum_coef;
        
        index++;

        [unroll]
        for (int k = 0; k < p_tex_ofs.x; k++)
        {
            stex0 = stex0 + step;
            stex1 = stex1 - step;
            stex2 = stex2 + step;
            stex3 = stex3 - step;
            
            col0 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex0);
            col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex1);
            col2 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex2);
            col3 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, stex3);
               
            _tmp0.xyz = CB_Coefficients[index].xyz * col0.w;
            sum = mad(col0.xyz, _tmp0, sum);
            sum_coef = _tmp0.xyz + sum_coef;
            _tmp0.xyz = CB_Coefficients[index].xyz * col1.w;
            sum = mad(col1.xyz, _tmp0, sum);
            sum_coef = _tmp0.xyz + sum_coef;
            _tmp0.xyz = CB_Coefficients[index].xyz * col2.w;
            sum = mad(col2.xyz, _tmp0, sum);
            sum_coef = _tmp0.xyz + sum_coef;
            _tmp0.xyz = CB_Coefficients[index].xyz * col3.w;
            sum = mad(col3.xyz, _tmp0, sum);
            sum_coef = _tmp0.xyz + sum_coef;
            index++;
        }
    }    
    
    o_color.xyz = (sum * rcp(sum_coef)) * p_color.xyz;
    o_color.w = 1.0;
    
    return o_color;
}
