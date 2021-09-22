// Win32Project1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "lesson_005.h"
#include "CELLWinApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CELL::CELLWinApp* app = new CELL::CELLWinApp();
	if (!app->createwindow(hInstance, 800, 600))
	{
		delete app;
		return -1;
	}
	app->main(0,0);
	delete app;

	return 0;
}
