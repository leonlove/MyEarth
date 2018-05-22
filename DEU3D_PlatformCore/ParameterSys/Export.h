#ifndef PARAMETER_SYS_EXPORT_H_696FABF2_A868_4810_B9E3_575DB1D9963B_INCLUDE
#define PARAMETER_SYS_EXPORT_H_696FABF2_A868_4810_B9E3_575DB1D9963B_INCLUDE

#ifdef WIN32

#pragma warning( disable : 4250 )

#ifdef PARAMETERSYS_EXPORTS
    #define PARAM_EXPORT    __declspec(dllexport)
#else
    #define PARAM_EXPORT    __declspec(dllimport)
#endif
#else
    #define PARAM_EXPORT
#endif

#endif

