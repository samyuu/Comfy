#pragma once
#include <memory>

template <class T>
using UniquePtr = _STD unique_ptr<T>;

template<class _Ty, class... _Types, _STD enable_if_t<!_STD is_array_v<_Ty>, int> = 0>
_NODISCARD inline UniquePtr<_Ty> MakeUnique(_Types&&... _Args)
{
	return (UniquePtr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...)));
}

template<class _Ty, _STD enable_if_t<_STD is_array_v<_Ty> && _STD extent_v<_Ty> == 0, int> = 0> 
_NODISCARD inline UniquePtr<_Ty> MakeUnique(size_t _Size)
{
	typedef _STD remove_extent_t<_Ty> _Elem;
	return (UniquePtr<_Ty>(new _Elem[_Size]()));
}

template<class _Ty, class... _Types, _STD enable_if_t<_STD extent_v<_Ty> != 0, int> = 0>
void MakeUnique(_Types&&...) = delete;

template <class T>
using RefPtr = _STD shared_ptr<T>;

template<class _Ty, class... _Types> 
_NODISCARD inline RefPtr<_Ty> MakeRef(_Types&&... _Args)
{
	return (_STD shared_ptr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...)));
}

template <class T>
using WeakPtr = _STD weak_ptr<T>;
