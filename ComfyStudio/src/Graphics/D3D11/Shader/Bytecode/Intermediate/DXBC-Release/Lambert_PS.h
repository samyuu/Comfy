#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer SceneConstantData
// {
//
//   struct
//   {
//       
//       struct
//       {
//           
//           float4x4 IrradianceRGB[3]; // Offset:    0
//           float4 LightColors[4];     // Offset:  192
//
//       } IBL;                         // Offset:    0
//       float4x4 View;                 // Offset:  256
//       float4x4 ViewProjection;       // Offset:  320
//       float4x4 LightSpace;           // Offset:  384
//       float4 EyePosition;            // Offset:  448
//       
//       struct
//       {
//           
//           float4 Ambient;            // Offset:  464
//           float4 Diffuse;            // Offset:  480
//           float4 Specular;           // Offset:  496
//           float4 Direction;          // Offset:  512
//
//       } CharaLight;                  // Offset:  464
//       
//       struct
//       {
//           
//           float4 Ambient;            // Offset:  528
//           float4 Diffuse;            // Offset:  544
//           float4 Specular;           // Offset:  560
//           float4 Direction;          // Offset:  576
//
//       } StageLight;                  // Offset:  528
//       float2 TexelRenderResolution;  // Offset:  592
//       float2 RenderResolution;       // Offset:  600
//       
//       struct
//       {
//           
//           float4 Time;               // Offset:  608
//           float4 TimeSin;            // Offset:  624
//           float4 TimeCos;            // Offset:  640
//
//       } RenderTime;                  // Offset:  608
//       
//       struct
//       {
//           
//           float4 Parameters;         // Offset:  656
//           float4 Color;              // Offset:  672
//
//       } DepthFog;                    // Offset:  656
//       float4 ShadowAmbient;          // Offset:  688
//       float4 OneMinusShadowAmbient;  // Offset:  704
//       float1 ShadowExponent;         // Offset:  720
//       float1 SubsurfaceScatteringParameter;// Offset:  724
//       uint DebugFlags;               // Offset:  728
//       uint Padding;                  // Offset:  732
//       float4 DebugValue;             // Offset:  736
//
//   } CB_Scene;                        // Offset:    0 Size:   752
//
// }
//
// cbuffer ObjectConstantData
// {
//
//   struct
//   {
//       
//       float4x4 Model;                // Offset:    0
//       float4x4 ModelView;            // Offset:   64
//       float4x4 ModelViewProjection;  // Offset:  128
//       
//       struct
//       {
//           
//           float4x4 DiffuseTextureTransform;// Offset:  192
//           float4x4 AmbientTextureTransform;// Offset:  256
//           float4 FresnelCoefficient; // Offset:  320
//           float4 Diffuse;            // Offset:  336
//           float4 Ambient;            // Offset:  352
//           float4 Specular;           // Offset:  368
//           float4 Emission;           // Offset:  384
//           float2 Shininess;          // Offset:  400
//           float1 Intensity;          // Offset:  408
//           float1 BumpDepth;          // Offset:  412
//
//       } Material;                    // Offset:  192
//       float4 MorphWeight;            // Offset:  416
//       uint ShaderFlags;              // Offset:  432
//       uint DiffuseRGTC1;             // Offset:  436
//       uint DiffuseScreenTexture;     // Offset:  440
//       uint AmbientTextureType;       // Offset:  444
//
//   } CB_Object;                       // Offset:    0 Size:   448
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// DiffuseSampler                    sampler      NA          NA             s0      1 
// AmbientSampler                    sampler      NA          NA             s1      1 
// ScreenReflectionSampler           sampler      NA          NA            s15      1 
// DiffuseTexture                    texture  float4          2d             t0      1 
// AmbientTexture                    texture  float4          2d             t1      1 
// ShadowMap                         texture   float          2d            t19      1 
// ESMGauss                          texture   float          2d            t21      1 
// SceneConstantData                 cbuffer      NA          NA            cb0      1 
// ObjectConstantData                cbuffer      NA          NA            cb1      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float       
// NORMAL                   0   xyzw        1     NONE   float       
// TANGENT                  0   xyzw        2     NONE   float       
// TANGENT                  1   xyzw        3     NONE   float       
// BINORMAL                 0   xyzw        4     NONE   float       
// EYE                      0   xyzw        5     NONE   float       
// TEXCOORD                 0   xy          6     NONE   float   xy  
// TEXCOORD                 1     zw        6     NONE   float     zw
// TEXCOORD                 2   xyzw        7     NONE   float   xyz 
// COLOR                    0   xyzw        8     NONE   float   xyzw
// COLOR                    1   xyzw        9     NONE   float       
// REFLECT                  0   xyzw       10     NONE   float       
// FOG                      0   x          11     NONE   float   x   
// POSITION                 0   xyzw       12     NONE   float       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   xyzw        0   TARGET   float   xyzw
//
ps_4_0
dcl_constantbuffer CB0[46], immediateIndexed
dcl_constantbuffer CB1[28], immediateIndexed
dcl_sampler s0, mode_default
dcl_sampler s1, mode_default
dcl_sampler s15, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_resource_texture2d (float,float,float,float) t19
dcl_resource_texture2d (float,float,float,float) t21
dcl_input_ps linear v6.xy
dcl_input_ps linear v6.zw
dcl_input_ps linear v7.xyz
dcl_input_ps linear v8.xyzw
dcl_input_ps linear v11.x
dcl_output o0.xyzw
dcl_temps 7
and r0.xyzw, cb1[27].xxxx, l(2, 4, 0x00008000, 2048)
if_nz r0.x
  sample r1.xyzw, v6.xyxx, t0.xyzw, s0
else 
  mov r1.xyzw, cb1[21].xyzw
endif 
if_nz r0.y
  sample r2.xyzw, v6.zwzz, t1.xyzw, s1
  add r3.xyz, -r1.xyzx, r2.xyzx
  mad r3.xyz, r2.wwww, r3.xyzx, r1.xyzx
  mul r4.xyzw, r1.xyzw, r2.xyzw
  add r5.xyz, r1.xyzx, r2.xyzx
  ieq r6.xyz, cb1[27].wwww, l(1, 2, 3, 0)
  add r0.x, r2.w, l(0.004000)
  div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
  mul r2.xyz, r0.xxxx, r2.xyzx
  mul r2.xyz, r1.xyzx, r2.xyzx
  movc r1.xyz, r6.zzzz, r2.xyzx, r1.xyzx
  mov r5.w, r4.w
  movc r2.xyzw, r6.yyyy, r5.xyzw, r1.xyzw
  movc r2.xyzw, r6.xxxx, r4.xyzw, r2.xyzw
  mov r3.w, r1.w
  movc r1.xyzw, cb1[27].wwww, r2.xyzw, r3.xyzw
endif 
if_nz r0.z
  sample r2.xyzw, v7.xyxx, t21.xyzw, s15
  add r0.x, r2.x, -v7.z
  mul r0.x, r0.x, cb0[45].x
  exp r0.x, r0.x
  min r0.x, r0.x, l(1.000000)
  sample r2.xyzw, v7.xyxx, t19.xyzw, s15
  max r0.x, r0.x, r2.x
  mad r2.xyzw, r0.xxxx, cb0[44].xyzw, cb0[43].xyzw
  mul r1.xyzw, r1.xyzw, r2.xyzw
endif 
mul r2.xyzw, r1.xyzw, v8.xyzw
mad r1.xyzw, -v8.xyzw, r1.xyzw, cb0[42].xyzw
mov r3.x, v11.x
mov r3.w, l(0)
mad r1.xyzw, r3.xxxw, r1.xyzw, r2.xyzw
movc r0.xyzw, r0.wwww, r1.xyzw, r2.xyzw
and r1.x, cb1[27].x, l(512)
ine r1.x, r1.x, l(0)
add r1.y, r0.w, l(-0.500000)
lt r1.y, r1.y, l(0.000000)
and r1.x, r1.x, r1.y
discard_nz r1.x
mov o0.xyzw, r0.xyzw
ret 
// Approximately 49 instruction slots used
#endif

const BYTE Lambert_PS_Bytecode[] =
{
     68,  88,  66,  67, 189, 189, 
    197, 190, 100, 163,  36,  63, 
    207, 199, 170, 166, 200,  97, 
    169, 126,   1,   0,   0,   0, 
    120,  16,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     16,   8,   0,   0, 188,   9, 
      0,   0, 240,   9,   0,   0, 
    252,  15,   0,   0,  82,  68, 
     69,  70, 212,   7,   0,   0, 
      2,   0,   0,   0, 200,   1, 
      0,   0,   9,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    255, 255,   0,   1,   0,   0, 
    172,   7,   0,   0,  60,   1, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     75,   1,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  90,   1,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 114,   1, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  13,   0,   0,   0, 
    129,   1,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   1,   0,   0,   0, 
      1,   0,   0,   0,  13,   0, 
      0,   0, 144,   1,   0,   0, 
      2,   0,   0,   0,   5,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,  19,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 154,   1, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
     21,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    163,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 181,   1,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  68, 105, 
    102, 102, 117, 115, 101,  83, 
     97, 109, 112, 108, 101, 114, 
      0,  65, 109,  98, 105, 101, 
    110, 116,  83,  97, 109, 112, 
    108, 101, 114,   0,  83,  99, 
    114, 101, 101, 110,  82, 101, 
    102, 108, 101,  99, 116, 105, 
    111, 110,  83,  97, 109, 112, 
    108, 101, 114,   0,  68, 105, 
    102, 102, 117, 115, 101,  84, 
    101, 120, 116, 117, 114, 101, 
      0,  65, 109,  98, 105, 101, 
    110, 116,  84, 101, 120, 116, 
    117, 114, 101,   0,  83, 104, 
     97, 100, 111, 119,  77,  97, 
    112,   0,  69,  83,  77,  71, 
     97, 117, 115, 115,   0,  83, 
     99, 101, 110, 101,  67, 111, 
    110, 115, 116,  97, 110, 116, 
     68,  97, 116,  97,   0,  79, 
     98, 106, 101,  99, 116,  67, 
    111, 110, 115, 116,  97, 110, 
    116,  68,  97, 116,  97,   0, 
    163,   1,   0,   0,   1,   0, 
      0,   0, 248,   1,   0,   0, 
    240,   2,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    181,   1,   0,   0,   1,   0, 
      0,   0, 156,   5,   0,   0, 
    192,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     16,   2,   0,   0,   0,   0, 
      0,   0, 240,   2,   0,   0, 
      2,   0,   0,   0, 140,   5, 
      0,   0,   0,   0,   0,   0, 
     67,  66,  95,  83,  99, 101, 
    110, 101,   0,  73,  66,  76, 
      0,  73, 114, 114,  97, 100, 
    105,  97, 110,  99, 101,  82, 
     71,  66,   0, 171,   3,   0, 
      3,   0,   4,   0,   4,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  76, 105, 103, 104, 
    116,  67, 111, 108, 111, 114, 
    115,   0,   1,   0,   3,   0, 
      1,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     29,   2,   0,   0,  44,   2, 
      0,   0,   0,   0,   0,   0, 
     60,   2,   0,   0,  72,   2, 
      0,   0, 192,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     64,   0,   0,   0,   2,   0, 
     88,   2,   0,   0,  86, 105, 
    101, 119,   0, 171, 171, 171, 
      3,   0,   3,   0,   4,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  86, 105, 
    101, 119,  80, 114, 111, 106, 
    101,  99, 116, 105, 111, 110, 
      0,  76, 105, 103, 104, 116, 
     83, 112,  97,  99, 101,   0, 
     69, 121, 101,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     67, 104,  97, 114,  97,  76, 
    105, 103, 104, 116,   0,  65, 
    109,  98, 105, 101, 110, 116, 
      0,  68, 105, 102, 102, 117, 
    115, 101,   0,  83, 112, 101, 
     99, 117, 108,  97, 114,   0, 
     68, 105, 114, 101,  99, 116, 
    105, 111, 110,   0, 171, 171, 
    219,   2,   0,   0, 192,   2, 
      0,   0,   0,   0,   0,   0, 
    227,   2,   0,   0, 192,   2, 
      0,   0,  16,   0,   0,   0, 
    235,   2,   0,   0, 192,   2, 
      0,   0,  32,   0,   0,   0, 
    244,   2,   0,   0, 192,   2, 
      0,   0,  48,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     16,   0,   0,   0,   4,   0, 
      0,   3,   0,   0,  83, 116, 
     97, 103, 101,  76, 105, 103, 
    104, 116,   0,  84, 101, 120, 
    101, 108,  82, 101, 110, 100, 
    101, 114,  82, 101, 115, 111, 
    108, 117, 116, 105, 111, 110, 
      0, 171, 171, 171,   1,   0, 
      3,   0,   1,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  82, 101, 110, 100, 
    101, 114,  82, 101, 115, 111, 
    108, 117, 116, 105, 111, 110, 
      0,  82, 101, 110, 100, 101, 
    114,  84, 105, 109, 101,   0, 
     84, 105, 109, 101,   0,  84, 
    105, 109, 101,  83, 105, 110, 
      0,  84, 105, 109, 101,  67, 
    111, 115,   0, 171, 171, 171, 
    144,   3,   0,   0, 192,   2, 
      0,   0,   0,   0,   0,   0, 
    149,   3,   0,   0, 192,   2, 
      0,   0,  16,   0,   0,   0, 
    157,   3,   0,   0, 192,   2, 
      0,   0,  32,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     12,   0,   0,   0,   3,   0, 
    168,   3,   0,   0,  68, 101, 
    112, 116, 104,  70, 111, 103, 
      0,  80,  97, 114,  97, 109, 
    101, 116, 101, 114, 115,   0, 
     67, 111, 108, 111, 114,   0, 
    171, 171, 229,   3,   0,   0, 
    192,   2,   0,   0,   0,   0, 
      0,   0, 240,   3,   0,   0, 
    192,   2,   0,   0,  16,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,   8,   0,   0,   0, 
      2,   0, 248,   3,   0,   0, 
     83, 104,  97, 100, 111, 119, 
     65, 109,  98, 105, 101, 110, 
    116,   0,  79, 110, 101,  77, 
    105, 110, 117, 115,  83, 104, 
     97, 100, 111, 119,  65, 109, 
     98, 105, 101, 110, 116,   0, 
     83, 104,  97, 100, 111, 119, 
     69, 120, 112, 111, 110, 101, 
    110, 116,   0, 171,   1,   0, 
      3,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  83, 117,  98, 115, 
    117, 114, 102,  97,  99, 101, 
     83,  99,  97, 116, 116, 101, 
    114, 105, 110, 103,  80,  97, 
    114,  97, 109, 101, 116, 101, 
    114,   0,  68, 101,  98, 117, 
    103,  70, 108,  97, 103, 115, 
      0, 171, 171, 171,   0,   0, 
     19,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  80,  97, 100, 100, 
    105, 110, 103,   0,  68, 101, 
     98, 117, 103,  86,  97, 108, 
    117, 101,   0, 171,  25,   2, 
      0,   0, 112,   2,   0,   0, 
      0,   0,   0,   0, 128,   2, 
      0,   0, 136,   2,   0,   0, 
      0,   1,   0,   0, 152,   2, 
      0,   0, 136,   2,   0,   0, 
     64,   1,   0,   0, 167,   2, 
      0,   0, 136,   2,   0,   0, 
    128,   1,   0,   0, 178,   2, 
      0,   0, 192,   2,   0,   0, 
    192,   1,   0,   0, 208,   2, 
      0,   0,  48,   3,   0,   0, 
    208,   1,   0,   0,  64,   3, 
      0,   0,  48,   3,   0,   0, 
     16,   2,   0,   0,  75,   3, 
      0,   0, 100,   3,   0,   0, 
     80,   2,   0,   0, 116,   3, 
      0,   0, 100,   3,   0,   0, 
     88,   2,   0,   0, 133,   3, 
      0,   0, 204,   3,   0,   0, 
     96,   2,   0,   0, 220,   3, 
      0,   0,  16,   4,   0,   0, 
    144,   2,   0,   0,  32,   4, 
      0,   0, 192,   2,   0,   0, 
    176,   2,   0,   0,  46,   4, 
      0,   0, 192,   2,   0,   0, 
    192,   2,   0,   0,  68,   4, 
      0,   0,  84,   4,   0,   0, 
    208,   2,   0,   0, 100,   4, 
      0,   0,  84,   4,   0,   0, 
    212,   2,   0,   0, 130,   4, 
      0,   0, 144,   4,   0,   0, 
    216,   2,   0,   0, 160,   4, 
      0,   0, 144,   4,   0,   0, 
    220,   2,   0,   0, 168,   4, 
      0,   0, 192,   2,   0,   0, 
    224,   2,   0,   0,   5,   0, 
      0,   0,   1,   0, 188,   0, 
      0,   0,  18,   0, 180,   4, 
      0,   0, 180,   5,   0,   0, 
      0,   0,   0,   0, 192,   1, 
      0,   0,   2,   0,   0,   0, 
    156,   7,   0,   0,   0,   0, 
      0,   0,  67,  66,  95,  79, 
     98, 106, 101,  99, 116,   0, 
     77, 111, 100, 101, 108,   0, 
     77, 111, 100, 101, 108,  86, 
    105, 101, 119,   0,  77, 111, 
    100, 101, 108,  86, 105, 101, 
    119,  80, 114, 111, 106, 101, 
     99, 116, 105, 111, 110,   0, 
     77,  97, 116, 101, 114, 105, 
     97, 108,   0,  68, 105, 102, 
    102, 117, 115, 101,  84, 101, 
    120, 116, 117, 114, 101,  84, 
    114,  97, 110, 115, 102, 111, 
    114, 109,   0,  65, 109,  98, 
    105, 101, 110, 116,  84, 101, 
    120, 116, 117, 114, 101,  84, 
    114,  97, 110, 115, 102, 111, 
    114, 109,   0,  70, 114, 101, 
    115, 110, 101, 108,  67, 111, 
    101, 102, 102, 105,  99, 105, 
    101, 110, 116,   0,  69, 109, 
    105, 115, 115, 105, 111, 110, 
      0,  83, 104, 105, 110, 105, 
    110, 101, 115, 115,   0,  73, 
    110, 116, 101, 110, 115, 105, 
    116, 121,   0,  66, 117, 109, 
    112,  68, 101, 112, 116, 104, 
      0, 171, 171, 171, 235,   5, 
      0,   0, 136,   2,   0,   0, 
      0,   0,   0,   0,   3,   6, 
      0,   0, 136,   2,   0,   0, 
     64,   0,   0,   0,  27,   6, 
      0,   0, 192,   2,   0,   0, 
    128,   0,   0,   0, 227,   2, 
      0,   0, 192,   2,   0,   0, 
    144,   0,   0,   0, 219,   2, 
      0,   0, 192,   2,   0,   0, 
    160,   0,   0,   0, 235,   2, 
      0,   0, 192,   2,   0,   0, 
    176,   0,   0,   0,  46,   6, 
      0,   0, 192,   2,   0,   0, 
    192,   0,   0,   0,  55,   6, 
      0,   0, 100,   3,   0,   0, 
    208,   0,   0,   0,  65,   6, 
      0,   0,  84,   4,   0,   0, 
    216,   0,   0,   0,  75,   6, 
      0,   0,  84,   4,   0,   0, 
    220,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,  56,   0, 
      0,   0,  10,   0,  88,   6, 
      0,   0,  77, 111, 114, 112, 
    104,  87, 101, 105, 103, 104, 
    116,   0,  83, 104,  97, 100, 
    101, 114,  70, 108,  97, 103, 
    115,   0,  68, 105, 102, 102, 
    117, 115, 101,  82,  71,  84, 
     67,  49,   0,  68, 105, 102, 
    102, 117, 115, 101,  83,  99, 
    114, 101, 101, 110,  84, 101, 
    120, 116, 117, 114, 101,   0, 
     65, 109,  98, 105, 101, 110, 
    116,  84, 101, 120, 116, 117, 
    114, 101,  84, 121, 112, 101, 
      0, 171, 171, 171, 190,   5, 
      0,   0, 136,   2,   0,   0, 
      0,   0,   0,   0, 196,   5, 
      0,   0, 136,   2,   0,   0, 
     64,   0,   0,   0, 206,   5, 
      0,   0, 136,   2,   0,   0, 
    128,   0,   0,   0, 226,   5, 
      0,   0, 208,   6,   0,   0, 
    192,   0,   0,   0, 224,   6, 
      0,   0, 192,   2,   0,   0, 
    160,   1,   0,   0, 236,   6, 
      0,   0, 144,   4,   0,   0, 
    176,   1,   0,   0, 248,   6, 
      0,   0, 144,   4,   0,   0, 
    180,   1,   0,   0,   5,   7, 
      0,   0, 144,   4,   0,   0, 
    184,   1,   0,   0,  26,   7, 
      0,   0, 144,   4,   0,   0, 
    188,   1,   0,   0,   5,   0, 
      0,   0,   1,   0, 112,   0, 
      0,   0,   9,   0,  48,   7, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  49,  48,  46,  49,   0, 
     73,  83,  71,  78, 164,   1, 
      0,   0,  14,   0,   0,   0, 
      8,   0,   0,   0,  88,   1, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0, 100,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     15,   0,   0,   0, 107,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     15,   0,   0,   0, 107,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
     15,   0,   0,   0, 115,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
     15,   0,   0,   0, 124,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
     15,   0,   0,   0, 128,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
      3,   3,   0,   0, 128,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
     12,  12,   0,   0, 128,   1, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   7,   0,   0,   0, 
     15,   7,   0,   0, 137,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   8,   0,   0,   0, 
     15,  15,   0,   0, 137,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   9,   0,   0,   0, 
     15,   0,   0,   0, 143,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  10,   0,   0,   0, 
     15,   0,   0,   0, 151,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  11,   0,   0,   0, 
      1,   1,   0,   0, 155,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  12,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  80,  79,  83,  73,  84, 
     73,  79,  78,   0,  78,  79, 
     82,  77,  65,  76,   0,  84, 
     65,  78,  71,  69,  78,  84, 
      0,  66,  73,  78,  79,  82, 
     77,  65,  76,   0,  69,  89, 
     69,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0,  67, 
     79,  76,  79,  82,   0,  82, 
     69,  70,  76,  69,  67,  84, 
      0,  70,  79,  71,   0,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  97, 114, 
    103, 101, 116,   0, 171, 171, 
     83,  72,  68,  82,   4,   6, 
      0,   0,  64,   0,   0,   0, 
    129,   1,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,  46,   0, 
      0,   0,  89,   0,   0,   4, 
     70, 142,  32,   0,   1,   0, 
      0,   0,  28,   0,   0,   0, 
     90,   0,   0,   3,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     90,   0,   0,   3,   0,  96, 
     16,   0,   1,   0,   0,   0, 
     90,   0,   0,   3,   0,  96, 
     16,   0,  15,   0,   0,   0, 
     88,  24,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,  19,   0, 
      0,   0,  85,  85,   0,   0, 
     88,  24,   0,   4,   0, 112, 
     16,   0,  21,   0,   0,   0, 
     85,  85,   0,   0,  98,  16, 
      0,   3,  50,  16,  16,   0, 
      6,   0,   0,   0,  98,  16, 
      0,   3, 194,  16,  16,   0, 
      6,   0,   0,   0,  98,  16, 
      0,   3, 114,  16,  16,   0, 
      7,   0,   0,   0,  98,  16, 
      0,   3, 242,  16,  16,   0, 
      8,   0,   0,   0,  98,  16, 
      0,   3,  18,  16,  16,   0, 
     11,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      0,   0,   0,   0, 104,   0, 
      0,   2,   7,   0,   0,   0, 
      1,   0,   0,  11, 242,   0, 
     16,   0,   0,   0,   0,   0, 
      6, 128,  32,   0,   1,   0, 
      0,   0,  27,   0,   0,   0, 
      2,  64,   0,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
      0, 128,   0,   0,   0,   8, 
      0,   0,  31,   0,   4,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  69,   0,   0,   9, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  16,  16,   0, 
      6,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  18,   0,   0,   1, 
     54,   0,   0,   6, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,  21,   0,   0,   0, 
     21,   0,   0,   1,  31,   0, 
      4,   3,  26,   0,  16,   0, 
      0,   0,   0,   0,  69,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0, 230,  26, 
     16,   0,   6,   0,   0,   0, 
     70, 126,  16,   0,   1,   0, 
      0,   0,   0,  96,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   8, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     50,   0,   0,   9, 114,   0, 
     16,   0,   3,   0,   0,   0, 
    246,  15,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7, 242,   0, 
     16,   0,   4,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   7, 114,   0,  16,   0, 
      5,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  32,   0,   0,  11, 
    114,   0,  16,   0,   6,   0, 
      0,   0, 246, 143,  32,   0, 
      1,   0,   0,   0,  27,   0, 
      0,   0,   2,  64,   0,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0, 111,  18, 
    131,  59,  14,   0,   0,  10, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   2,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      2,   0,   0,   0,  56,   0, 
      0,   7, 114,   0,  16,   0, 
      2,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  55,   0,   0,   9, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      6,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
    130,   0,  16,   0,   5,   0, 
      0,   0,  58,   0,  16,   0, 
      4,   0,   0,   0,  55,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0,  86,   5, 
     16,   0,   6,   0,   0,   0, 
     70,  14,  16,   0,   5,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  55,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0,   6,   0, 
     16,   0,   6,   0,   0,   0, 
     70,  14,  16,   0,   4,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      3,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     55,   0,   0,  10, 242,   0, 
     16,   0,   1,   0,   0,   0, 
    246, 143,  32,   0,   1,   0, 
      0,   0,  27,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,  21,   0, 
      0,   1,  31,   0,   4,   3, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  69,   0,   0,   9, 
    242,   0,  16,   0,   2,   0, 
      0,   0,  70,  16,  16,   0, 
      7,   0,   0,   0,  70, 126, 
     16,   0,  21,   0,   0,   0, 
      0,  96,  16,   0,  15,   0, 
      0,   0,   0,   0,   0,   8, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,  42,  16, 
     16, 128,  65,   0,   0,   0, 
      7,   0,   0,   0,  56,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,  45,   0,   0,   0, 
     25,   0,   0,   5,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  51,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     69,   0,   0,   9, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  16,  16,   0,   7,   0, 
      0,   0,  70, 126,  16,   0, 
     19,   0,   0,   0,   0,  96, 
     16,   0,  15,   0,   0,   0, 
     52,   0,   0,   7,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,  50,   0, 
      0,  11, 242,   0,  16,   0, 
      2,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  44,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  43,   0,   0,   0, 
     56,   0,   0,   7, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  21,   0, 
      0,   1,  56,   0,   0,   7, 
    242,   0,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  30, 
     16,   0,   8,   0,   0,   0, 
     50,   0,   0,  11, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  30,  16, 128,  65,   0, 
      0,   0,   8,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  42,   0, 
      0,   0,  54,   0,   0,   5, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  10,  16,  16,   0, 
     11,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     50,   0,   0,   9, 242,   0, 
     16,   0,   1,   0,   0,   0, 
      6,  12,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
     55,   0,   0,   9, 242,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
      1,   0,   0,   8,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     10, 128,  32,   0,   1,   0, 
      0,   0,  27,   0,   0,   0, 
      1,  64,   0,   0,   0,   2, 
      0,   0,  39,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   7,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0, 191,  49,   0, 
      0,   7,  34,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     13,   0,   4,   3,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5, 242,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 116,   0, 
      0,   0,  49,   0,   0,   0, 
      7,   0,   0,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     20,   0,   0,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,   5,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0
};
