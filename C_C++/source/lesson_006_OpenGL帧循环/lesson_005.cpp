// Win32Project1.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "lesson_005.h"
#include "CELLWinApp.h"

//lesson_005 ����openGL������
//1. �����ƽ̨����ͷ�ļ�������ƽ̨��صĺ궨��
//2. ����OpenGL�����Ļ�������

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
