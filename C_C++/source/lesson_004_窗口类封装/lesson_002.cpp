// Win32Project1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "lesson_002.h"
#include "CELLWinApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CELL::CELLWinApp winApp;
	winApp.createwindow(hInstance, 800, 600);

	winApp.main(0,0);

	return 0;
}
