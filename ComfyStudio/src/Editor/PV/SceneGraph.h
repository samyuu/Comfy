#pragma once
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/ObjSet.h"

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
		const Graphics::Obj* Obj;
		bool IsReflection;
	};

	struct ObjSetResource
	{
		RefPtr<Graphics::ObjSet> ObjSet;
		EntityTag Tag;
	};

	struct SceneGraph
	{
		std::vector<ObjSetResource> LoadedObjSets;

		// TODO: Store UniquePtr / RefPtr so other objects can hold references
		std::vector<ObjectEntity> Entities;

		inline ObjSetResource& LoadObjSet(const RefPtr<Graphics::ObjSet>& objSet, EntityTag tag)
		{
			LoadedObjSets.emplace_back(ObjSetResource { objSet, tag });
			return LoadedObjSets.back();
		}

		inline ObjectEntity& AddEntityFromObj(const Graphics::Obj& obj, EntityTag tag)
		{
			ObjectEntity entity = {};
			entity.Name = obj.Name;
			entity.Tag = tag;
			entity.Obj = &obj;
			entity.IsVisible = true;
			entity.Transform = Graphics::Transform(vec3(0.0f));
			
			Entities.push_back(entity);
			return Entities.back();
		}
	};
}
