#ifndef CONSTANTINPUTS_HLSL
#define CONSTANTINPUTS_HLSL

#include "Structs/SceneData.hlsl"
#include "Structs/ObjectData.hlsl"

cbuffer SceneConstantData : register(b0)
{
    SceneData CB_Scene;
};

cbuffer ObjectConstantData : register(b1)
{
    ObjectData CB_Object;
};

#endif /* CONSTANTINPUTS_HLSL */
