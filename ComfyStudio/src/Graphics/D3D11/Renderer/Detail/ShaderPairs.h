#pragma once
#include "Graphics/D3D11/Shader/Bytecode/ShaderBytecode.h"

namespace Comfy::Graphics::D3D11
{
	struct ShaderPairs
	{
		ShaderPair DebugMaterial = { DebugMaterial_VS(), DebugMaterial_PS(), "Renderer3D::DebugMaterial" };
		ShaderPair SilhouetteOutline = { FullscreenQuad_VS(), SilhouetteOutline_PS(), "Renderer3D::SilhouetteOutline" };

		ShaderPair BlinnPerFrag = { BlinnPerFrag_VS(), BlinnPerFrag_PS(), "Renderer3D::BlinnPerFrag" };
		ShaderPair BlinnPerVert = { BlinnPerVert_VS(), BlinnPerVert_PS(), "Renderer3D::BlinnPerVert" };
		ShaderPair ClothAniso = { ClothDefault_VS(), ClothAniso_PS(), "Renderer3D::ClothAniso" };
		ShaderPair ClothDefault = { ClothDefault_VS(), ClothDefault_PS(), "Renderer3D::ClothDefault" };
		ShaderPair Constant = { Constant_VS(), Constant_PS(), "Renderer3D::Constant" };
		ShaderPair DepthThreshold = { FullscreenQuad_VS(), DepthThreshold_PS(), "Renderer3D::DepthThreshold" };
		ShaderPair EyeBall = { EyeBall_VS(), EyeBall_PS(), "Renderer3D::EyeBall" };
		ShaderPair EyeLens = { EyeLens_VS(), EyeLens_PS(), "Renderer3D::EyeLens" };
		ShaderPair Floor = { Floor_VS(), Floor_PS(), "Renderer3D::Floor" };
		ShaderPair GlassEye = { GlassEye_VS(), GlassEye_PS(), "Renderer3D::GlassEye" };
		ShaderPair HairAniso = { HairDefault_VS(), HairAniso_PS(), "Renderer3D::HairAniso" };
		ShaderPair HairDefault = { HairDefault_VS(), HairDefault_PS(), "Renderer3D::HairDefault" };
		ShaderPair ESMFilterMin = { FullscreenQuad_VS(), ESMFilterMin_PS(), "Renderer3D::ESMFilterMin" };
		ShaderPair ESMFilterErosion = { FullscreenQuad_VS(), ESMFilterErosion_PS(), "Renderer3D::ESMFilterErosion" };
		ShaderPair ESMGauss = { FullscreenQuad_VS(), ESMGauss_PS(), "Renderer3D::ESMGauss" };
		ShaderPair ExposureMinify = { FullscreenQuad_VS(), ExposureMinify_PS(), "Renderer3D::ExposureMinify" };
		ShaderPair ExposureMeasure = { FullscreenQuad_VS(), ExposureMeasure_PS(), "Renderer3D::ExposureMeasure" };
		ShaderPair ExposureAverage = { FullscreenQuad_VS(), ExposureAverage_PS(), "Renderer3D::ExposureAverage" };
		ShaderPair ImgFilter = { FullscreenQuad_VS(), ImgFilter_PS(), "Renderer3D::ImgFilter" };
		ShaderPair ImgFilterBlur = { FullscreenQuad_VS(), ImgFilterBlur_PS(), "Renderer3D::ImgFilterBlur" };
		ShaderPair ItemBlinn = { ItemBlinn_VS(), ItemBlinn_PS(), "Renderer3D::ItemBlinn" };
		ShaderPair Lambert = { Lambert_VS(), Lambert_PS(), "Renderer3D::Lambert" };
		ShaderPair LensFlare = { LensFlare_VS(), LensFlare_PS(), "Renderer3D::LensFlare" };
		ShaderPair Silhouette = { Silhouette_VS(), Silhouette_PS(), "Renderer3D::Silhouette" };
		ShaderPair PPGauss = { FullscreenQuad_VS(), PPGauss_PS(), "Renderer3D::PPGauss" };
		ShaderPair ReduceTex = { FullscreenQuad_VS(), ReduceTex_PS(), "Renderer3D::ReduceTex" };
		ShaderPair SkinDefault = { SkinDefault_VS(), SkinDefault_PS(), "Renderer3D::SkinDefault" };
		ShaderPair SkyDefault = { SkyDefault_VS(), SkyDefault_PS(), "Renderer3D::SkyDefault" };
		ShaderPair SolidBlack = { PositionTransform_VS(), SolidBlack_PS(), "Renderer3D::SolidBlack" };
		ShaderPair SolidWhite = { PositionTransform_VS(), SolidWhite_PS(), "Renderer3D::SolidWhite" };
		ShaderPair SSSFilterCopy = { FullscreenQuad_VS(), SSSFilterCopy_PS(), "Renderer3D::SSSFilterCopy" };
		ShaderPair SSSFilterMin = { FullscreenQuad_VS(), SSSFilterMin_PS(), "Renderer3D::SSSFilterMin" };
		ShaderPair SSSFilterGauss2D = { FullscreenQuad_VS(), SSSFilterGauss2D_PS(), "Renderer3D::SSSFilterGauss2D" };
		ShaderPair SSSSkin = { SSSSkin_VS(), SSSSkin_PS(), "Renderer3D::SSSSkin" };
		ShaderPair SSSSkinConst = { PositionTransform_VS(), SSSSkinConst_PS(), "Renderer3D::SSSSkinConst" };
		ShaderPair StageBlinn = { StageBlinn_VS(), StageBlinn_PS(), "Renderer3D::StageBlinn" };
		ShaderPair Sun = { LensFlare_VS(), Sun_PS(), "Renderer3D::Sun" };
		ShaderPair Tights = { Tights_VS(), Tights_PS(), "Renderer3D::Tights" };
		ShaderPair ToneMap = { FullscreenQuad_VS(), ToneMap_PS(), "Renderer3D::ToneMap" };
		ShaderPair Water = { Water_VS(), Water_PS(), "Renderer3D::Water" };
	};
}
