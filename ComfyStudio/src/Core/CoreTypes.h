#pragma once
#include <string>
#include <array>
#include <vector>

using String = std::string;
using WideString = std::wstring;

template <class _Ty, size_t _Size>
using Array = std::array<_Ty, _Size>;

template <class _Ty>
using Vector = std::vector<_Ty>;
