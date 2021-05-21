#pragma once
//问题1：#pragma once是什么？有什么用？

#define CELL_PLATFORM_WIN32     1
#define CELL_PLATFORM_LINUX     2
#define CELL_PLATFORM_APPLE     3
#define CELL_PLATFORM_ANDRIOD   4

#define CELL_PATH_LENGHT        1024
#define CELL_CALL               __stdcall 
//问题2：__stdcall是什么？有什么用？

#define CELL_PLATFORM   CELL_PLATFORM_WIN32

#if CELL_PLATFORM == CELL_PLATFORM_WIN32
#   include <stdio.h>
#   include <stdlib.h>
#   include <wchar.h>
#   include <tchar.h>
#   include <windows.h>


//#   include <glew/glew.h>
#   include <gl/GL.h>
#   include <gl/GLu.h>

//问题3：window里__declspec(dllexport)是什么？有什么用？Linux却没有呢？
#ifdef CELLBIGMAP3D_EXPORTS
#define BIGMAP3D_API __declspec(dllexport)
#else
#define BIGMAP3D_API __declspec(dllimport)
#endif


#   define  HDISPLAY    HDC
#   define  HWINDOW     HWND
#   define  HLIBRARY    HMODULE
#	define	INSTANCE    HINSTANCE

    typedef const char* STRING;


#elif CELL_PLATFORM == CELL_PLATFORM_LINUX || CELL_PLATFORM == CELL_PLATFORM_ANDRIOD

#   include <stdio.h>
#   include <stdlib.h>
#   include <string.h>
#   include <wchar.h>
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include "glew.h"
#   include <GL/gl.h>
#   include <GL/glx.h>
#   include <GL/glu.h>
#   include <dlfcn.h>
#   include <pthread.h>
#   include <semaphore.h>
#   include <unistd.h>
#   include <sys/time.h>

#ifdef CELLBIGMAP3D_EXPORTS
#define BIGMAP3D_API
#else
#define BIGMAP3D_API

#   define  HDISPLAY    void*
#   define  HWINDOW     void*
#   define  HLIBRARY    void*

#endif

#endif