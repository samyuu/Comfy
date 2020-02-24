#pragma once
#include "Graphics/Direct3D/D3D_Shader.h"

#define DECLARE_BYTECODE_GETTER(shader)	BytecodeBlob shader();
#define DEFINE_BYTECODE_GETTER(shader)	BytecodeBlob shader() { return { ::shader##_Bytecode, sizeof(::shader##_Bytecode) }; }

// NOTE: Public interface
namespace Graphics
{
	DECLARE_BYTECODE_GETTER(ImGuiDefault_VS);
	DECLARE_BYTECODE_GETTER(ImGuiDefault_PS);
	DECLARE_BYTECODE_GETTER(ImGuiCustom_PS);
	DECLARE_BYTECODE_GETTER(Sprite_VS);
	DECLARE_BYTECODE_GETTER(Sprite_PS);
	DECLARE_BYTECODE_GETTER(Debug_VS);
	DECLARE_BYTECODE_GETTER(Debug_PS);
	DECLARE_BYTECODE_GETTER(FullscreenQuad_VS);
	DECLARE_BYTECODE_GETTER(SilhouetteOutline_PS);
	DECLARE_BYTECODE_GETTER(BlinnPerFrag_VS);
	DECLARE_BYTECODE_GETTER(BlinnPerFrag_PS);
	DECLARE_BYTECODE_GETTER(BlinnPerVert_VS);
	DECLARE_BYTECODE_GETTER(BlinnPerVert_PS);
	DECLARE_BYTECODE_GETTER(ClothAniso_PS);
	DECLARE_BYTECODE_GETTER(ClothDefault_VS);
	DECLARE_BYTECODE_GETTER(ClothDefault_PS);
	DECLARE_BYTECODE_GETTER(Constant_VS);
	DECLARE_BYTECODE_GETTER(Constant_PS);
	DECLARE_BYTECODE_GETTER(DepthThreshold_PS);
	DECLARE_BYTECODE_GETTER(ESMFilter_PS);
	DECLARE_BYTECODE_GETTER(ESMGauss_PS);
	DECLARE_BYTECODE_GETTER(EyeBall_VS);
	DECLARE_BYTECODE_GETTER(EyeBall_PS);
	DECLARE_BYTECODE_GETTER(EyeLens_VS);
	DECLARE_BYTECODE_GETTER(EyeLens_PS);
	DECLARE_BYTECODE_GETTER(Floor_VS);
	DECLARE_BYTECODE_GETTER(Floor_PS);
	DECLARE_BYTECODE_GETTER(GlassEye_VS);
	DECLARE_BYTECODE_GETTER(GlassEye_PS);
	DECLARE_BYTECODE_GETTER(HairAniso_PS);
	DECLARE_BYTECODE_GETTER(HairDefault_VS);
	DECLARE_BYTECODE_GETTER(HairDefault_PS);
	DECLARE_BYTECODE_GETTER(ImgFilter_PS);
	DECLARE_BYTECODE_GETTER(ImgFilterBlur_PS);
	DECLARE_BYTECODE_GETTER(ItemBlinn_VS);
	DECLARE_BYTECODE_GETTER(ItemBlinn_PS);
	DECLARE_BYTECODE_GETTER(PositionTransform_VS);
	DECLARE_BYTECODE_GETTER(Lambert_VS);
	DECLARE_BYTECODE_GETTER(Lambert_PS);
	DECLARE_BYTECODE_GETTER(Silhouette_VS);
	DECLARE_BYTECODE_GETTER(Silhouette_PS);
	DECLARE_BYTECODE_GETTER(PPGauss_PS);
	DECLARE_BYTECODE_GETTER(ReduceTex_PS);
	DECLARE_BYTECODE_GETTER(SkinDefault_VS);
	DECLARE_BYTECODE_GETTER(SkinDefault_PS);
	DECLARE_BYTECODE_GETTER(SkyDefault_VS);
	DECLARE_BYTECODE_GETTER(SkyDefault_PS);
	DECLARE_BYTECODE_GETTER(SolidBlack_PS);
	DECLARE_BYTECODE_GETTER(SolidWhite_PS);
	DECLARE_BYTECODE_GETTER(SSSFilter_PS);
	DECLARE_BYTECODE_GETTER(SSSSkin_VS);
	DECLARE_BYTECODE_GETTER(SSSSkin_PS);
	DECLARE_BYTECODE_GETTER(SSSSkinConst_PS);
	DECLARE_BYTECODE_GETTER(StageBlinn_VS);
	DECLARE_BYTECODE_GETTER(StageBlinn_PS);
	DECLARE_BYTECODE_GETTER(Tights_VS);
	DECLARE_BYTECODE_GETTER(Tights_PS);
	DECLARE_BYTECODE_GETTER(ToneMap_PS);
	DECLARE_BYTECODE_GETTER(Water_VS);
	DECLARE_BYTECODE_GETTER(Water_PS);
}

// NOTE: Implementation included in the cpp file
#ifdef SHADER_BYTECODE_IMPLEMENTATION

#include SHADER_BYTECODE_FILE(ImGuiDefault_VS.h)
#include SHADER_BYTECODE_FILE(ImGuiDefault_PS.h)
#include SHADER_BYTECODE_FILE(ImGuiCustom_PS.h)
#include SHADER_BYTECODE_FILE(Sprite_VS.h)
#include SHADER_BYTECODE_FILE(Sprite_PS.h)
#include SHADER_BYTECODE_FILE(Debug_VS.h)
#include SHADER_BYTECODE_FILE(Debug_PS.h)
#include SHADER_BYTECODE_FILE(FullscreenQuad_VS.h)
#include SHADER_BYTECODE_FILE(SilhouetteOutline_PS.h)
#include SHADER_BYTECODE_FILE(BlinnPerFrag_VS.h)
#include SHADER_BYTECODE_FILE(BlinnPerFrag_PS.h)
#include SHADER_BYTECODE_FILE(BlinnPerVert_VS.h)
#include SHADER_BYTECODE_FILE(BlinnPerVert_PS.h)
#include SHADER_BYTECODE_FILE(ClothAniso_PS.h)
#include SHADER_BYTECODE_FILE(ClothDefault_VS.h)
#include SHADER_BYTECODE_FILE(ClothDefault_PS.h)
#include SHADER_BYTECODE_FILE(Constant_VS.h)
#include SHADER_BYTECODE_FILE(Constant_PS.h)
#include SHADER_BYTECODE_FILE(DepthThreshold_PS.h)
#include SHADER_BYTECODE_FILE(ESMFilter_PS.h)
#include SHADER_BYTECODE_FILE(ESMGauss_PS.h)
#include SHADER_BYTECODE_FILE(EyeBall_VS.h)
#include SHADER_BYTECODE_FILE(EyeBall_PS.h)
#include SHADER_BYTECODE_FILE(EyeLens_VS.h)
#include SHADER_BYTECODE_FILE(EyeLens_PS.h)
#include SHADER_BYTECODE_FILE(Floor_VS.h)
#include SHADER_BYTECODE_FILE(Floor_PS.h)
#include SHADER_BYTECODE_FILE(GlassEye_VS.h)
#include SHADER_BYTECODE_FILE(GlassEye_PS.h)
#include SHADER_BYTECODE_FILE(HairAniso_PS.h)
#include SHADER_BYTECODE_FILE(HairDefault_VS.h)
#include SHADER_BYTECODE_FILE(HairDefault_PS.h)
#include SHADER_BYTECODE_FILE(ImgFilter_PS.h)
#include SHADER_BYTECODE_FILE(ImgFilterBlur_PS.h)
#include SHADER_BYTECODE_FILE(ItemBlinn_VS.h)
#include SHADER_BYTECODE_FILE(ItemBlinn_PS.h)
#include SHADER_BYTECODE_FILE(PositionTransform_VS.h)
#include SHADER_BYTECODE_FILE(Lambert_VS.h)
#include SHADER_BYTECODE_FILE(Lambert_PS.h)
#include SHADER_BYTECODE_FILE(Silhouette_VS.h)
#include SHADER_BYTECODE_FILE(Silhouette_PS.h)
#include SHADER_BYTECODE_FILE(PPGauss_PS.h)
#include SHADER_BYTECODE_FILE(ReduceTex_PS.h)
#include SHADER_BYTECODE_FILE(SkinDefault_VS.h)
#include SHADER_BYTECODE_FILE(SkinDefault_PS.h)
#include SHADER_BYTECODE_FILE(SkyDefault_VS.h)
#include SHADER_BYTECODE_FILE(SkyDefault_PS.h)
#include SHADER_BYTECODE_FILE(SolidBlack_PS.h)
#include SHADER_BYTECODE_FILE(SolidWhite_PS.h)
#include SHADER_BYTECODE_FILE(SSSFilter_PS.h)
#include SHADER_BYTECODE_FILE(SSSSkin_VS.h)
#include SHADER_BYTECODE_FILE(SSSSkin_PS.h)
#include SHADER_BYTECODE_FILE(SSSSkinConst_PS.h)
#include SHADER_BYTECODE_FILE(StageBlinn_VS.h)
#include SHADER_BYTECODE_FILE(StageBlinn_PS.h)
#include SHADER_BYTECODE_FILE(Tights_VS.h)
#include SHADER_BYTECODE_FILE(Tights_PS.h)
#include SHADER_BYTECODE_FILE(ToneMap_PS.h)
#include SHADER_BYTECODE_FILE(Water_VS.h)
#include SHADER_BYTECODE_FILE(Water_PS.h)

namespace Graphics
{
	DEFINE_BYTECODE_GETTER(ImGuiDefault_VS);
	DEFINE_BYTECODE_GETTER(ImGuiDefault_PS);
	DEFINE_BYTECODE_GETTER(ImGuiCustom_PS);
	DEFINE_BYTECODE_GETTER(Sprite_VS);
	DEFINE_BYTECODE_GETTER(Sprite_PS);
	DEFINE_BYTECODE_GETTER(Debug_VS);
	DEFINE_BYTECODE_GETTER(Debug_PS);
	DEFINE_BYTECODE_GETTER(FullscreenQuad_VS);
	DEFINE_BYTECODE_GETTER(SilhouetteOutline_PS);
	DEFINE_BYTECODE_GETTER(BlinnPerFrag_VS);
	DEFINE_BYTECODE_GETTER(BlinnPerFrag_PS);
	DEFINE_BYTECODE_GETTER(BlinnPerVert_VS);
	DEFINE_BYTECODE_GETTER(BlinnPerVert_PS);
	DEFINE_BYTECODE_GETTER(ClothAniso_PS);
	DEFINE_BYTECODE_GETTER(ClothDefault_VS);
	DEFINE_BYTECODE_GETTER(ClothDefault_PS);
	DEFINE_BYTECODE_GETTER(Constant_VS);
	DEFINE_BYTECODE_GETTER(Constant_PS);
	DEFINE_BYTECODE_GETTER(DepthThreshold_PS);
	DEFINE_BYTECODE_GETTER(ESMFilter_PS);
	DEFINE_BYTECODE_GETTER(ESMGauss_PS);
	DEFINE_BYTECODE_GETTER(EyeBall_VS);
	DEFINE_BYTECODE_GETTER(EyeBall_PS);
	DEFINE_BYTECODE_GETTER(EyeLens_VS);
	DEFINE_BYTECODE_GETTER(EyeLens_PS);
	DEFINE_BYTECODE_GETTER(Floor_VS);
	DEFINE_BYTECODE_GETTER(Floor_PS);
	DEFINE_BYTECODE_GETTER(GlassEye_VS);
	DEFINE_BYTECODE_GETTER(GlassEye_PS);
	DEFINE_BYTECODE_GETTER(HairAniso_PS);
	DEFINE_BYTECODE_GETTER(HairDefault_VS);
	DEFINE_BYTECODE_GETTER(HairDefault_PS);
	DEFINE_BYTECODE_GETTER(ImgFilter_PS);
	DEFINE_BYTECODE_GETTER(ImgFilterBlur_PS);
	DEFINE_BYTECODE_GETTER(ItemBlinn_VS);
	DEFINE_BYTECODE_GETTER(ItemBlinn_PS);
	DEFINE_BYTECODE_GETTER(PositionTransform_VS);
	DEFINE_BYTECODE_GETTER(Lambert_VS);
	DEFINE_BYTECODE_GETTER(Lambert_PS);
	DEFINE_BYTECODE_GETTER(Silhouette_VS);
	DEFINE_BYTECODE_GETTER(Silhouette_PS);
	DEFINE_BYTECODE_GETTER(PPGauss_PS);
	DEFINE_BYTECODE_GETTER(ReduceTex_PS);
	DEFINE_BYTECODE_GETTER(SkinDefault_VS);
	DEFINE_BYTECODE_GETTER(SkinDefault_PS);
	DEFINE_BYTECODE_GETTER(SkyDefault_VS);
	DEFINE_BYTECODE_GETTER(SkyDefault_PS);
	DEFINE_BYTECODE_GETTER(SolidBlack_PS);
	DEFINE_BYTECODE_GETTER(SolidWhite_PS);
	DEFINE_BYTECODE_GETTER(SSSFilter_PS);
	DEFINE_BYTECODE_GETTER(SSSSkin_VS);
	DEFINE_BYTECODE_GETTER(SSSSkin_PS);
	DEFINE_BYTECODE_GETTER(SSSSkinConst_PS);
	DEFINE_BYTECODE_GETTER(StageBlinn_VS);
	DEFINE_BYTECODE_GETTER(StageBlinn_PS);
	DEFINE_BYTECODE_GETTER(Tights_VS);
	DEFINE_BYTECODE_GETTER(Tights_PS);
	DEFINE_BYTECODE_GETTER(ToneMap_PS);
	DEFINE_BYTECODE_GETTER(Water_VS);
	DEFINE_BYTECODE_GETTER(Water_PS);
}

#endif /* SHADER_BYTECODE_IMPLEMENTATION */
