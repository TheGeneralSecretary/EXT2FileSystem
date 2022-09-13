#pragma once

#ifdef BUILD_DEBUG
#ifdef BUILD_PLATFORM_WINDOWS
#define BUILD_DEBUGBREAK() __debugbreak()	
#else
#error "platform doesnt support debugbreak"
#endif

#define BUILD_ENABLE_ASSERTS
#else
#define BUILD_DEBUGBREAK()
#endif

#ifdef BUILD_ENABLE_ASSERTS
#define BUILD_ASSERT(x, msg) { if(!(x)) { LOGGER_ERROR("Error: {}, File: {}, Line: {}", msg, __FILE__, __LINE__); BUILD_DEBUGBREAK(); } }
#else
#define BUILD_ASSERT(...)
#endif
