#pragma once

namespace Comfy::Hacks
{
	// NOTE: This is without a doubt invoking undefined behavior but MSVC does not currently allow enabling RTTI for specified classes only
	//		 and from my testing with _MSV_VER=1916 this is reliable for both debug and release builds.

#if !defined(_MSC_VER)
	template <typename T>
	const void* GetVirtualFunctionTablePointer(const T& polymorphicObject)
	{
		static_assert(false, __FUNCTION__" is not guaranteed to work with different compilers such as when the vfptr is stored at the end of an object.");
		return nullptr;
	}
#else
	template <typename T>
	const void* GetVirtualFunctionTablePointer(const T& polymorphicObject)
	{
		static_assert(std::is_polymorphic_v<T> && sizeof(T) >= sizeof(void*), "Non-polymorphic objects do not have a virtual function table.");

		const auto vfptr = *reinterpret_cast<const void* const*>(&polymorphicObject);
		return vfptr;
	}
#endif

	template <typename BaseType>
	bool CompareVirtualFunctionTablePointers(const BaseType& polymorphicObjectA, const BaseType& polymorphicObjectB)
	{
		return (GetVirtualFunctionTablePointer(polymorphicObjectA) == GetVirtualFunctionTablePointer(polymorphicObjectB));
	}
}
