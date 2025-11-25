#pragma once
#ifdef _WIN32


#ifndef NOMINMAX
#define NOMINMAX
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#ifdef RTYPE_INCLUDE_WINDOWS_H
#include <windows.h>


#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef DrawText
#undef DrawText
#endif

#ifdef PlaySound
#undef PlaySound
#endif

#ifdef LoadLibrary
#undef LoadLibrary
#endif

#ifdef FreeLibrary
#undef FreeLibrary
#endif

#ifdef GetMessage
#undef GetMessage
#endif

#ifdef SendMessage
#undef SendMessage
#endif

#ifdef PostMessage
#undef PostMessage
#endif

#ifdef GetObject
#undef GetObject
#endif

#ifdef RGB
#undef RGB
#endif

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#endif // RTYPE_INCLUDE_WINDOWS_H

#endif // _WIN32
