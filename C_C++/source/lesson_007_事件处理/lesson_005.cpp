// Win32Project1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "lesson_005.h"
#include "CELLWinApp.h"

//lesson_005 创建openGL上下文
//1. 引入跨平台管理头文件，设置平台相关的宏定义
//2. 引入OpenGL下上文环境设置

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CELL::CELLWinApp winApp;
	if (!winApp.createwindow(hInstance, 800, 600))
	{
		return -1;
	}
	winApp.main(0,0);

	return 0;
}
