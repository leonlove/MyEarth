#ifndef PLATFORM_EXPORT_
#define PLATFORM_EXPORT_ 1


#ifdef PLATFORMCORE_EXPORTS
#define PLATFORM_EXPORT    __declspec(dllexport)
#else
#define PLATFORM_EXPORT    __declspec(dllimport)
#endif

#endif