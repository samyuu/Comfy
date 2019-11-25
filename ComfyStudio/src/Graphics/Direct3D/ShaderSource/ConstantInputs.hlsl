#include "SceneData.hlsl"
#include "Material.hlsl"
#include "ShaderFlags.hlsl"

// TODO: program.env[24].x
static const uint AmbientTextureType = 1;

// TODO: state.matrix.program[0]
// matrix IrradianceRed;
// TODO: state.matrix.program[1]
// matrix IrradianceGreen;
// TODO: state.matrix.program[2]
// matrix IrradianceBlue;
static const float3 Irradiance = float3(0.64, 0.49, 0.33);

// TODO: program.env[3]
static const float3 BlendColor = float3(1.0, 1.0, 1.0);

cbuffer SceneConstantData : register(b0)
{
    SceneData CB_Scene;
    float1 SceneCB_Padding[8];
};

cbuffer ObjectConstantData : register(b1)
{
    matrix CB_Model;
    Material CB_Material;
    uint CB_ShaderFlags;
    float ObjectCB_Padding[12];
};
