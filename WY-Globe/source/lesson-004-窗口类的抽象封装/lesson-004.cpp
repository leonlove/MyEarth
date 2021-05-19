// lesson-002.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "lesson-004.h"
#include "CELLWinApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	CELL::CELLWinApp app;
	app.createWindow(800, 600, hInstance);
	app.main(0, 0);
	return 0;
}
