#pragma once

template<class T> inline bool HasFlag(T a, T b) { return (static_cast<int>(a) & static_cast<int>(b)); }
template<class T> inline bool HasFlag(int a, T b) { return (a & static_cast<int>(b)); }

template<class T> inline T operator~ (T a) { return (T)~static_cast<int>(a); }
template<class T> inline T operator| (T a, T b) { return (T)(static_cast<int>(a) | static_cast<int>(b)); }
template<class T> inline T operator& (T a, T b) { return (T)(static_cast<int>(a) & static_cast<int>(b)); }
template<class T> inline T operator^ (T a, T b) { return (T)(static_cast<int>(a) ^ static_cast<int>(b)); }