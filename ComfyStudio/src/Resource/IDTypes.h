#pragma once
#include "Types.h"

namespace Comfy
{
	constexpr u32 InvalidResourceID = 0xFFFFFFFF;

#define DECLARE_ID_TYPE(typeName) enum class typeName : u32 { Invalid = InvalidResourceID };
#include "Detail/IDTypeDeclarations.h"
#undef DECLARE_ID_TYPE

	template <typename IDType>
	struct CachedResourceID
	{
		CachedResourceID() = default;
		CachedResourceID(IDType id) : ID(id) {};

		IDType ID;
		mutable u32 CachedIndex;

		operator IDType() const { return ID; };
	};

#define DECLARE_ID_TYPE(typeName) using Cached_##typeName = CachedResourceID<typeName>;
#include "Detail/IDTypeDeclarations.h"
#undef DECLARE_ID_TYPE
}
