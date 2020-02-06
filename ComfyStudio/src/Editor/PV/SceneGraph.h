#pragma once
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Graphics/Auth3D/ObjAnimationData.h"
#include "Database/TxpDB.h"

namespace Editor
{
	typedef int64_t EntityTag;

	struct Entity
	{
		std::string Name;

		// NOTE: Store user data for identification
		EntityTag Tag;

		bool IsVisible;
		Graphics::Transform Transform;
	};

	struct ObjectEntity : public Entity
	{
		const Graphics::Obj* Obj = nullptr;
		const Graphics::Obj* MorphObj = nullptr;

		bool IsReflection = false;
		bool SilhouetteOutline = false;

		UniquePtr<Graphics::ObjAnimationData> Animation = nullptr;
	};

	struct ObjSetResource
	{
		RefPtr<Graphics::ObjSet> ObjSet;
		EntityTag Tag;
	};

	struct SceneGraph
	{
		UniquePtr<Database::TxpDB> TxpDB = nullptr;

		std::vector<ObjSetResource> LoadedObjSets;
		std::vector<UniquePtr<ObjectEntity>> Entities;

		inline ObjSetResource& LoadObjSet(const RefPtr<Graphics::ObjSet>& objSet, EntityTag tag)
		{
			LoadedObjSets.emplace_back(ObjSetResource { objSet, tag });
			return LoadedObjSets.back();
		}

		inline ObjectEntity& AddEntityFromObj(const Graphics::Obj& obj, EntityTag tag)
		{
			auto entity = MakeUnique<ObjectEntity>();
			entity->Name = obj.Name;
			entity->Tag = tag;
			entity->IsVisible = true;
			entity->Transform = Graphics::Transform(vec3(0.0f));
			entity->Obj = &obj;
			entity->MorphObj = nullptr;
			entity->IsReflection = false;
			
			Entities.push_back(std::move(entity));
			return *Entities.back();
		}
	};
}
