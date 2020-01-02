#include "ComfyTextureID.h"
#include "Graphics/Direct3D/D3D_Texture.h"
#include "Graphics/Direct3D/D3D_RenderTarget.h"

using namespace Graphics;

ComfyTextureID::ComfyTextureID(const nullptr_t dummy)
	: ResourceView(dummy), IsCubeMap(false), IsRGTC(false)
{
}

ComfyTextureID::ComfyTextureID(const D3D_TextureResource& texture)
	: ResourceView(texture.GetResourceView()), IsCubeMap(texture.GetArraySize() == 6), IsRGTC(!IsCubeMap && texture.GetTextureFormat() == TextureFormat::RGTC2)
{
}

ComfyTextureID::ComfyTextureID(const D3D_RenderTarget& renderTarget) 
	: ResourceView(renderTarget.GetResourceView()), IsCubeMap(false), IsRGTC(false)
{

}

bool ComfyTextureID::operator==(const ComfyTextureID& other) const
{
	return ResourceView == other.ResourceView;
}

bool ComfyTextureID::operator!=(const ComfyTextureID& other) const
{
	return ResourceView != other.ResourceView;
}
