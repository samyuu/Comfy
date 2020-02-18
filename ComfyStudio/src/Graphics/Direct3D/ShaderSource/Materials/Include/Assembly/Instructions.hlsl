#ifndef INSTRUCTIONS_HLSL
#define INSTRUCTIONS_HLSL

#define FLOAT1_ZERO                 ((float1)0)
#define FLOAT2_ZERO                 ((float2)0)
#define FLOAT3_ZERO                 ((float3)0)
#define FLOAT4_ZERO                 ((float4)0)
#define FLOAT1_ONE                  ((float1)1)
#define FLOAT2_ONE                  ((float2)1)
#define FLOAT3_ONE                  ((float3)1)
#define FLOAT4_ONE                  ((float4)1)

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
#define TEX2D_06(result, texCoord)  result = TranslucencyTexture.Sample(TranslucencySampler, (texCoord).xy)
// TODO: simple_reflect...
#define TEX2D_15(result, texCoord)  result = ScreenReflectionTexture.Sample(ScreenReflectionSampler, (texCoord).xy)
#define TEX2D_16(result, texCoord)  result = SubsurfaceScatteringTexture.Sample(ScreenReflectionSampler, (texCoord).xy)
#define TEX2D_19(result, texCoord)  result = ESMFull.Sample(ScreenReflectionSampler, (texCoord).xy)

#define TEXCUBE_05(result, texCoord) result = ReflectionCubeMap.Sample( ReflectionSampler, (texCoord).xyz )
#define TEXCUBE_09(result, texCoord) result = CharacterLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_10(result, texCoord) result = SunLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_11(result, texCoord) result = ReflectLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_12(result, texCoord) result = ShadowLightMap.Sample( LightMapSampler, (texCoord).xyz )
#define TEXCUBE_13(result, texCoord) result = CharColorLightMap.Sample( LightMapSampler, (texCoord).xyz )

#define TXLCUBE_09(result, texCoord) result = CharacterLightMap.SampleLevel( LightMapSampler, (texCoord).xyz, (texCoord).w )

#ifdef COMFY_VS
#define RET                         return output
#endif /* COMFY_VS */

#ifdef COMFY_PS
#define RET                         return o_color
#endif /* COMFY_PS */

#endif /* INSTRUCTIONS_HLSL */
