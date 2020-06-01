#pragma once
#include "Tests/TestTask.h"
#include "FilePaths.h"

namespace Comfy::Sandbox::Tests::Game
{
	using namespace Graphics;

	class GameContext : NonCopyable
	{
	public:
		GameContext()
		{
			Renderer.Aet().SetSprGetter([&](const Aet::VideoSource& source) -> Render::TexSpr
			{
				for (auto sprSet : allAetAccessibleSprSets)
				{
					if (auto result = Render::SprSetNameStringSprGetter(source, sprSet->get()); result.Tex != nullptr && result.Spr != nullptr)
						return result;
				}

				return Render::NullSprGetter(source);
			});
		}

	public:
		TimeSpan Elapsed = TimeSpan::Zero();

	public:
		Render::Renderer2D Renderer = {};

	public:
		std::unique_ptr<Aet::AetSet> AetPS4Menu = IO::File::Load<Aet::AetSet>(FilePaths::AetPS4Menu);
		std::shared_ptr<Aet::Scene> AetPS4MenuMain = (AetPS4Menu != nullptr) ? AetPS4Menu->GetScenes().front() : nullptr;
		
		std::unique_ptr<SprSet> SprPS4Menu = IO::File::Load<SprSet>(FilePaths::SprPS4Menu);

	public:
		std::unique_ptr<SprSet> SprFont36 = IO::File::Load<SprSet>(FilePaths::SprFont36);
		
		std::unique_ptr<FontMap> FontMap = IO::File::Load<Graphics::FontMap>(FilePaths::FontMap);
		const BitmapFont* Font36 = FindFont36();

	private:
		std::array<std::unique_ptr<SprSet>*, 1> allAetAccessibleSprSets
		{
			&SprPS4Menu,
		};

	private:
		const BitmapFont* FindFont36() const
		{
			if (FontMap == nullptr)
				return nullptr;

			auto font36 = FontMap->FindFont(ivec2(36));
			if (font36 != nullptr && SprFont36 != nullptr)
				font36->Texture = SprFont36->TexSet->Textures.front();
			return font36;
		}
	};
}
