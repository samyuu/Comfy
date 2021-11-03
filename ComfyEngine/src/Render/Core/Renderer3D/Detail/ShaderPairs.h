#pragma once
#include "Render/D3D11/Shader/Bytecode/ShaderBytecode.h"

namespace Comfy::Render::Detail
{
	struct RendererShaderPairs
	{
		D3D11ShaderPair DebugMaterial = { GlobalD3D11, DebugMaterial_VS(), DebugMaterial_PS(), "Renderer3D::DebugMaterial" };
		D3D11ShaderPair SilhouetteOutline = { GlobalD3D11, FullscreenQuad_VS(), SilhouetteOutline_PS(), "Renderer3D::SilhouetteOutline" };

		D3D11ShaderPair BlinnPerFrag = { GlobalD3D11, BlinnPerFrag_VS(), BlinnPerFrag_PS(), "Renderer3D::BlinnPerFrag" };
		D3D11ShaderPair BlinnPerVert = { GlobalD3D11, BlinnPerVert_VS(), BlinnPerVert_PS(), "Renderer3D::BlinnPerVert" };
		D3D11ShaderPair ClothAniso = { GlobalD3D11, ClothDefault_VS(), ClothAniso_PS(), "Renderer3D::ClothAniso" };
		D3D11ShaderPair ClothDefault = { GlobalD3D11, ClothDefault_VS(), ClothDefault_PS(), "Renderer3D::ClothDefault" };
		D3D11ShaderPair Constant = { GlobalD3D11, Constant_VS(), Constant_PS(), "Renderer3D::Constant" };
		D3D11ShaderPair DepthThreshold = { GlobalD3D11, FullscreenQuad_VS(), DepthThreshold_PS(), "Renderer3D::DepthThreshold" };
		D3D11ShaderPair EyeBall = { GlobalD3D11, EyeBall_VS(), EyeBall_PS(), "Renderer3D::EyeBall" };
		D3D11ShaderPair EyeLens = { GlobalD3D11, EyeLens_VS(), EyeLens_PS(), "Renderer3D::EyeLens" };
		D3D11ShaderPair Floor = { GlobalD3D11, Floor_VS(), Floor_PS(), "Renderer3D::Floor" };
		D3D11ShaderPair GlassEye = { GlobalD3D11, GlassEye_VS(), GlassEye_PS(), "Renderer3D::GlassEye" };
		D3D11ShaderPair HairAniso = { GlobalD3D11, HairDefault_VS(), HairAniso_PS(), "Renderer3D::HairAniso" };
		D3D11ShaderPair HairDefault = { GlobalD3D11, HairDefault_VS(), HairDefault_PS(), "Renderer3D::HairDefault" };
		D3D11ShaderPair ESMFilterMin = { GlobalD3D11, FullscreenQuad_VS(), ESMFilterMin_PS(), "Renderer3D::ESMFilterMin" };
		D3D11ShaderPair ESMFilterErosion = { GlobalD3D11, FullscreenQuad_VS(), ESMFilterErosion_PS(), "Renderer3D::ESMFilterErosion" };
		D3D11ShaderPair ESMGauss = { GlobalD3D11, FullscreenQuad_VS(), ESMGauss_PS(), "Renderer3D::ESMGauss" };
		D3D11ShaderPair ExposureMinify = { GlobalD3D11, FullscreenQuad_VS(), ExposureMinify_PS(), "Renderer3D::ExposureMinify" };
		D3D11ShaderPair ExposureMeasure = { GlobalD3D11, FullscreenQuad_VS(), ExposureMeasure_PS(), "Renderer3D::ExposureMeasure" };
		D3D11ShaderPair ExposureAverage = { GlobalD3D11, FullscreenQuad_VS(), ExposureAverage_PS(), "Renderer3D::ExposureAverage" };
		D3D11ShaderPair ImgFilter = { GlobalD3D11, FullscreenQuad_VS(), ImgFilter_PS(), "Renderer3D::ImgFilter" };
		D3D11ShaderPair ImgFilterBlur = { GlobalD3D11, FullscreenQuad_VS(), ImgFilterBlur_PS(), "Renderer3D::ImgFilterBlur" };
		D3D11ShaderPair ItemBlinn = { GlobalD3D11, ItemBlinn_VS(), ItemBlinn_PS(), "Renderer3D::ItemBlinn" };
		D3D11ShaderPair Lambert = { GlobalD3D11, Lambert_VS(), Lambert_PS(), "Renderer3D::Lambert" };
		D3D11ShaderPair LensFlare = { GlobalD3D11, LensFlare_VS(), LensFlare_PS(), "Renderer3D::LensFlare" };
		D3D11ShaderPair Silhouette = { GlobalD3D11, Silhouette_VS(), Silhouette_PS(), "Renderer3D::Silhouette" };
		D3D11ShaderPair PPGauss = { GlobalD3D11, FullscreenQuad_VS(), PPGauss_PS(), "Renderer3D::PPGauss" };
		D3D11ShaderPair ReduceTex = { GlobalD3D11, FullscreenQuad_VS(), ReduceTex_PS(), "Renderer3D::ReduceTex" };
		D3D11ShaderPair SkinDefault = { GlobalD3D11, SkinDefault_VS(), SkinDefault_PS(), "Renderer3D::SkinDefault" };
		D3D11ShaderPair SkyDefault = { GlobalD3D11, SkyDefault_VS(), SkyDefault_PS(), "Renderer3D::SkyDefault" };
		D3D11ShaderPair SolidBlack = { GlobalD3D11, PositionTransform_VS(), SolidBlack_PS(), "Renderer3D::SolidBlack" };
		D3D11ShaderPair SolidWhite = { GlobalD3D11, PositionTransform_VS(), SolidWhite_PS(), "Renderer3D::SolidWhite" };
		D3D11ShaderPair SSSFilterCopy = { GlobalD3D11, FullscreenQuad_VS(), SSSFilterCopy_PS(), "Renderer3D::SSSFilterCopy" };
		D3D11ShaderPair SSSFilterMin = { GlobalD3D11, FullscreenQuad_VS(), SSSFilterMin_PS(), "Renderer3D::SSSFilterMin" };
		D3D11ShaderPair SSSFilterGauss2D = { GlobalD3D11, FullscreenQuad_VS(), SSSFilterGauss2D_PS(), "Renderer3D::SSSFilterGauss2D" };
		D3D11ShaderPair SSSSkin = { GlobalD3D11, SSSSkin_VS(), SSSSkin_PS(), "Renderer3D::SSSSkin" };
		D3D11ShaderPair SSSSkinConst = { GlobalD3D11, PositionTransform_VS(), SSSSkinConst_PS(), "Renderer3D::SSSSkinConst" };
		D3D11ShaderPair StageBlinn = { GlobalD3D11, StageBlinn_VS(), StageBlinn_PS(), "Renderer3D::StageBlinn" };
		D3D11ShaderPair Sun = { GlobalD3D11, LensFlare_VS(), Sun_PS(), "Renderer3D::Sun" };
		D3D11ShaderPair Tights = { GlobalD3D11, Tights_VS(), Tights_PS(), "Renderer3D::Tights" };
		D3D11ShaderPair ToneMap = { GlobalD3D11, FullscreenQuad_VS(), ToneMap_PS(), "Renderer3D::ToneMap" };
		D3D11ShaderPair Water = { GlobalD3D11, Water_VS(), Water_PS(), "Renderer3D::Water" };
	};
}
