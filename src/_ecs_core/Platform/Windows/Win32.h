#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// For some reason this constant is not define in Windows.h
constexpr int GWL_USERDATA = -21; // (Should be a C style macro)