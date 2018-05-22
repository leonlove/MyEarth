#ifndef IDPROVIDER_EXPORT_
#define IDPROVIDER_EXPORT_ 1

#ifdef WIN32
#ifdef IDPROVIDER_EXPORTS
    #define IPV_EXPORT    __declspec(dllexport)
#else
    #define IPV_EXPORT    __declspec(dllimport)
#endif
#else
    #define IPV_EXPORT
#endif

#endif
