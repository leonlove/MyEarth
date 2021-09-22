#include "stdafx.h"
#include "CELLWinApp.h"
#include "Resource.h"

namespace CELL {

	CELLWinApp::CELLWinApp()
	{
		_hWnd = 0;
	}

	CELLWinApp::~CELLWinApp()
	{
		_hWnd = 0;
	}

	bool CELLWinApp::createwindow(HINSTANCE hInstance, int width, int height)
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

		return TRUE;

	}

	void CELLWinApp::main(int argc, char **argv)
	{
		//消息循环
		MSG msg;

		// 主消息循环: 
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT CALLBACK CELLWinApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
			HDC hdc = BeginPaint(hWnd, &ps);
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