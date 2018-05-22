#ifndef COMMON_EXPORTS_
#define COMMON_EXPORTS_ 1

#ifdef WIN32
#ifdef COMMON_EXPORTS
#define CM_EXPORT    __declspec(dllexport)
#else
#define CM_EXPORT    __declspec(dllimport)
#endif
#else
#define CM_EXPORT
#endif

#endif