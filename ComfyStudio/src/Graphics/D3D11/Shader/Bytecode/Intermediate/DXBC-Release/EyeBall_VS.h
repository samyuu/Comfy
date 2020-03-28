#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
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
// ObjectConstantData                cbuffer      NA          NA            cb1      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyzw        0     NONE   float   xyzw
// NORMAL                   0   xyz         1     NONE   float       
// TANGENT                  0   xyzw        2     NONE   float       
// TEXCOORD                 0   xy          3     NONE   float       
// TEXCOORD                 1   xy          4     NONE   float       
// COLOR                    0   xyzw        5     NONE   float       
// COLOR                    1   xyzw        6     NONE   float       
// BLENDWEIGHT              0   xyzw        7     NONE   float       
// BLENDINDICES             0   xyzw        8     NONE   float       
// POSITION                 1   xyzw        9     NONE   float       
// NORMAL                   1   xyz        10     NONE   float       
// TANGENT                  1   xyzw       11     NONE   float       
// TEXCOORD                 4   xy         12     NONE   float       
// TEXCOORD                 5   xy         13     NONE   float       
// COLOR                    2   xyzw       14     NONE   float       
// COLOR                    3   xyzw       15     NONE   float       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// NORMAL                   0   xyzw        1     NONE   float   xyzw
// TANGENT                  0   xyzw        2     NONE   float   xyzw
// TANGENT                  1   xyzw        3     NONE   float   xyzw
// BINORMAL                 0   xyzw        4     NONE   float   xyzw
// EYE                      0   xyzw        5     NONE   float   xyzw
// TEXCOORD                 0   xy          6     NONE   float   xy  
// TEXCOORD                 1     zw        6     NONE   float     zw
// TEXCOORD                 2   xyzw        7     NONE   float   xyzw
// COLOR                    0   xyzw        8     NONE   float   xyzw
// COLOR                    1   xyzw        9     NONE   float   xyzw
// REFLECT                  0   xyzw       10     NONE   float   xyzw
// FOG                      0   x          11     NONE   float   x   
// POSITION                 0   xyzw       12     NONE   float   xyzw
//
vs_4_0
dcl_constantbuffer CB1[12], immediateIndexed
dcl_input v0.xyzw
dcl_output_siv o0.xyzw, position
dcl_output o1.xyzw
dcl_output o2.xyzw
dcl_output o3.xyzw
dcl_output o4.xyzw
dcl_output o5.xyzw
dcl_output o6.xy
dcl_output o6.zw
dcl_output o7.xyzw
dcl_output o8.xyzw
dcl_output o9.xyzw
dcl_output o10.xyzw
dcl_output o11.x
dcl_output o12.xyzw
dp4 o0.x, v0.xyzw, cb1[8].xyzw
dp4 o0.y, v0.xyzw, cb1[9].xyzw
dp4 o0.z, v0.xyzw, cb1[10].xyzw
dp4 o0.w, v0.xyzw, cb1[11].xyzw
mov o1.xyzw, l(0,0,0,0)
mov o2.xyzw, l(0,0,0,0)
mov o3.xyzw, l(0,0,0,0)
mov o4.xyzw, l(0,0,0,0)
mov o5.xyzw, l(0,0,0,0)
mov o6.xyzw, l(0,0,0,0)
mov o7.xyzw, l(0,0,0,0)
mov o8.xyzw, l(0,0,0,0)
mov o9.xyzw, l(0,0,0,0)
mov o10.xyzw, l(0,0,0,0)
mov o11.x, l(0)
mov o12.xyzw, l(0,0,0,0)
ret 
// Approximately 17 instruction slots used
#endif

const BYTE EyeBall_VS_Bytecode[] =
{
     68,  88,  66,  67, 192, 208, 
    217, 235,  25, 209, 145, 235, 
    248,   2, 199, 130,  61,  99, 
    166,  49,   1,   0,   0,   0, 
     16,  10,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     72,   3,   0,   0,  24,   5, 
      0,   0, 196,   6,   0,   0, 
    148,   9,   0,   0,  82,  68, 
     69,  70,  12,   3,   0,   0, 
      1,   0,   0,   0,  80,   0, 
      0,   0,   1,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    254, 255,   0,   1,   0,   0, 
    228,   2,   0,   0,  60,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     79,  98, 106, 101,  99, 116, 
     67, 111, 110, 115, 116,  97, 
    110, 116,  68,  97, 116,  97, 
      0, 171,  60,   0,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0, 192,   1,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 128,   0,   0,   0, 
      0,   0,   0,   0, 192,   1, 
      0,   0,   2,   0,   0,   0, 
    212,   2,   0,   0,   0,   0, 
      0,   0,  67,  66,  95,  79, 
     98, 106, 101,  99, 116,   0, 
     77, 111, 100, 101, 108,   0, 
      3,   0,   3,   0,   4,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  77, 111, 
    100, 101, 108,  86, 105, 101, 
    119,   0,  77, 111, 100, 101, 
    108,  86, 105, 101, 119,  80, 
    114, 111, 106, 101,  99, 116, 
    105, 111, 110,   0,  77,  97, 
    116, 101, 114, 105,  97, 108, 
      0,  68, 105, 102, 102, 117, 
    115, 101,  84, 101, 120, 116, 
    117, 114, 101,  84, 114,  97, 
    110, 115, 102, 111, 114, 109, 
      0,  65, 109,  98, 105, 101, 
    110, 116,  84, 101, 120, 116, 
    117, 114, 101,  84, 114,  97, 
    110, 115, 102, 111, 114, 109, 
      0,  70, 114, 101, 115, 110, 
    101, 108,  67, 111, 101, 102, 
    102, 105,  99, 105, 101, 110, 
    116,   0, 171, 171,   1,   0, 
      3,   0,   1,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  68, 105, 102, 102, 
    117, 115, 101,   0,  65, 109, 
     98, 105, 101, 110, 116,   0, 
     83, 112, 101,  99, 117, 108, 
     97, 114,   0,  69, 109, 105, 
    115, 115, 105, 111, 110,   0, 
     83, 104, 105, 110, 105, 110, 
    101, 115, 115,   0,   1,   0, 
      3,   0,   1,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  73, 110, 116, 101, 
    110, 115, 105, 116, 121,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     66, 117, 109, 112,  68, 101, 
    112, 116, 104,   0, 171, 171, 
    199,   0,   0,   0, 144,   0, 
      0,   0,   0,   0,   0,   0, 
    223,   0,   0,   0, 144,   0, 
      0,   0,  64,   0,   0,   0, 
    247,   0,   0,   0,  12,   1, 
      0,   0, 128,   0,   0,   0, 
     28,   1,   0,   0,  12,   1, 
      0,   0, 144,   0,   0,   0, 
     36,   1,   0,   0,  12,   1, 
      0,   0, 160,   0,   0,   0, 
     44,   1,   0,   0,  12,   1, 
      0,   0, 176,   0,   0,   0, 
     53,   1,   0,   0,  12,   1, 
      0,   0, 192,   0,   0,   0, 
     62,   1,   0,   0,  72,   1, 
      0,   0, 208,   0,   0,   0, 
     88,   1,   0,   0, 100,   1, 
      0,   0, 216,   0,   0,   0, 
    116,   1,   0,   0, 100,   1, 
      0,   0, 220,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     56,   0,   0,   0,  10,   0, 
    128,   1,   0,   0,  77, 111, 
    114, 112, 104,  87, 101, 105, 
    103, 104, 116,   0,  83, 104, 
     97, 100, 101, 114,  70, 108, 
     97, 103, 115,   0,   0,   0, 
     19,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  68, 105, 102, 102, 
    117, 115, 101,  82,  71,  84, 
     67,  49,   0,  68, 105, 102, 
    102, 117, 115, 101,  83,  99, 
    114, 101, 101, 110,  84, 101, 
    120, 116, 117, 114, 101,   0, 
     65, 109,  98, 105, 101, 110, 
    116,  84, 101, 120, 116, 117, 
    114, 101,  84, 121, 112, 101, 
      0, 171, 171, 171, 138,   0, 
      0,   0, 144,   0,   0,   0, 
      0,   0,   0,   0, 160,   0, 
      0,   0, 144,   0,   0,   0, 
     64,   0,   0,   0, 170,   0, 
      0,   0, 144,   0,   0,   0, 
    128,   0,   0,   0, 190,   0, 
      0,   0, 248,   1,   0,   0, 
    192,   0,   0,   0,   8,   2, 
      0,   0,  12,   1,   0,   0, 
    160,   1,   0,   0,  20,   2, 
      0,   0,  32,   2,   0,   0, 
    176,   1,   0,   0,  48,   2, 
      0,   0,  32,   2,   0,   0, 
    180,   1,   0,   0,  61,   2, 
      0,   0,  32,   2,   0,   0, 
    184,   1,   0,   0,  82,   2, 
      0,   0,  32,   2,   0,   0, 
    188,   1,   0,   0,   5,   0, 
      0,   0,   1,   0, 112,   0, 
      0,   0,   9,   0, 104,   2, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  49,  48,  46,  49,   0, 
     73,  83,  71,  78, 200,   1, 
      0,   0,  16,   0,   0,   0, 
      8,   0,   0,   0, 136,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,  15,   0,   0, 145,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      7,   0,   0,   0, 152,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     15,   0,   0,   0, 160,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0, 160,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
      3,   0,   0,   0, 169,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
     15,   0,   0,   0, 169,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   6,   0,   0,   0, 
     15,   0,   0,   0, 175,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   7,   0,   0,   0, 
     15,   0,   0,   0, 187,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   8,   0,   0,   0, 
     15,   0,   0,   0, 136,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   9,   0,   0,   0, 
     15,   0,   0,   0, 145,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  10,   0,   0,   0, 
      7,   0,   0,   0, 152,   1, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  11,   0,   0,   0, 
     15,   0,   0,   0, 160,   1, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  12,   0,   0,   0, 
      3,   0,   0,   0, 160,   1, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  13,   0,   0,   0, 
      3,   0,   0,   0, 169,   1, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  14,   0,   0,   0, 
     15,   0,   0,   0, 169,   1, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  15,   0,   0,   0, 
     15,   0,   0,   0,  80,  79, 
     83,  73,  84,  73,  79,  78, 
      0,  78,  79,  82,  77,  65, 
     76,   0,  84,  65,  78,  71, 
     69,  78,  84,   0,  84,  69, 
     88,  67,  79,  79,  82,  68, 
      0,  67,  79,  76,  79,  82, 
      0,  66,  76,  69,  78,  68, 
     87,  69,  73,  71,  72,  84, 
      0,  66,  76,  69,  78,  68, 
     73,  78,  68,  73,  67,  69, 
     83,   0,  79,  83,  71,  78, 
    164,   1,   0,   0,  14,   0, 
      0,   0,   8,   0,   0,   0, 
     88,   1,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  15,   0,   0,   0, 
    107,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,  15,   0,   0,   0, 
    107,   1,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,  15,   0,   0,   0, 
    115,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   4,   0, 
      0,   0,  15,   0,   0,   0, 
    124,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   5,   0, 
      0,   0,  15,   0,   0,   0, 
    128,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   6,   0, 
      0,   0,   3,  12,   0,   0, 
    128,   1,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   6,   0, 
      0,   0,  12,   3,   0,   0, 
    128,   1,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   7,   0, 
      0,   0,  15,   0,   0,   0, 
    137,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   8,   0, 
      0,   0,  15,   0,   0,   0, 
    137,   1,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   9,   0, 
      0,   0,  15,   0,   0,   0, 
    143,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  10,   0, 
      0,   0,  15,   0,   0,   0, 
    151,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  11,   0, 
      0,   0,   1,  14,   0,   0, 
    155,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  12,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  80,  79,  83, 
     73,  84,  73,  79,  78,   0, 
     78,  79,  82,  77,  65,  76, 
      0,  84,  65,  78,  71,  69, 
     78,  84,   0,  66,  73,  78, 
     79,  82,  77,  65,  76,   0, 
     69,  89,  69,   0,  84,  69, 
     88,  67,  79,  79,  82,  68, 
      0,  67,  79,  76,  79,  82, 
      0,  82,  69,  70,  76,  69, 
     67,  84,   0,  70,  79,  71, 
      0,  80,  79,  83,  73,  84, 
     73,  79,  78,   0,  83,  72, 
     68,  82, 200,   2,   0,   0, 
     64,   0,   1,   0, 178,   0, 
      0,   0,  89,   0,   0,   4, 
     70, 142,  32,   0,   1,   0, 
      0,   0,  12,   0,   0,   0, 
     95,   0,   0,   3, 242,  16, 
     16,   0,   0,   0,   0,   0, 
    103,   0,   0,   4, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      2,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      3,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      4,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      5,   0,   0,   0, 101,   0, 
      0,   3,  50,  32,  16,   0, 
      6,   0,   0,   0, 101,   0, 
      0,   3, 194,  32,  16,   0, 
      6,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      7,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      8,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      9,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
     10,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
     11,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
     12,   0,   0,   0,  17,   0, 
      0,   8,  18,  32,  16,   0, 
      0,   0,   0,   0,  70,  30, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     17,   0,   0,   8,  34,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  30,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      1,   0,   0,   0,   9,   0, 
      0,   0,  17,   0,   0,   8, 
     66,  32,  16,   0,   0,   0, 
      0,   0,  70,  30,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
     10,   0,   0,   0,  17,   0, 
      0,   8, 130,  32,  16,   0, 
      0,   0,   0,   0,  70,  30, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,  11,   0,   0,   0, 
     54,   0,   0,   8, 242,  32, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   8, 
    242,  32,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 242,  32, 
     16,   0,   4,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   8, 
    242,  32,  16,   0,   5,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
      6,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 242,  32, 
     16,   0,   7,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   8, 
    242,  32,  16,   0,   8,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
      9,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 242,  32, 
     16,   0,  10,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,  11,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
     12,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  83,  84, 
     65,  84, 116,   0,   0,   0, 
     17,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
