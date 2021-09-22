#include "stdafx.h"
#include "CELLWinApp.h"
#include "Resource.h"

namespace CELL {

	CELLWinApp::CELLWinApp()
	{
		_hWnd		= 0;
	}

	CELLWinApp::~CELLWinApp()
	{
		_hWnd = 0;
	}

	bool CELLWinApp::createwindow(INSTANCE hInstance, int width, int height)
	{
		//1. 注册窗口
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = 0;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = _T("CELL.BigMap");
		wcex.hIconSm = 0;

		RegisterClassExW(&wcex);

		//2. 创建窗口
		_hWnd = CreateWindowW(_T("CELL.BigMap")
			,_T("BigMap")
			, WS_OVERLAPPEDWINDOW
			, CW_USEDEFAULT
			, 0
			, CW_USEDEFAULT
			, 0
			, nullptr
			, nullptr
			, hInstance
			, nullptr);

		if (!_hWnd)
		{
			return FALSE;
		}

		//3. 更新显示窗口
		ShowWindow(_hWnd, SW_SHOW);
		UpdateWindow(_hWnd);

		HDISPLAY hDC = GetDC(_hWnd);
		if (!_contextGL.init(_hWnd, hDC))
		{
			DestroyWindow(_hWnd);
			return false;
		}
		
		return TRUE;

	}

	void CELLWinApp::main(int argc, char **argv)
	{
		//消息循环
		MSG msg = {0};
#if 0
		// 主消息循环: 
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#else
		// 主消息循环: 
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			render();
		}
#endif
		_contextGL.shutdown();
	}

	void CELLWinApp::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1, 0, 0, 1);
		
		_contextGL.swapBuffer();
	}

	LRESULT CALLBACK CELLWinApp::WndProc(HWINDOW hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 分析菜单选择: 
			switch (wmId)
			{
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDISPLAY hdc = BeginPaint(hWnd, &ps);
			// TODO: 在此处添加使用 hdc 的任何绘图代码...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

}