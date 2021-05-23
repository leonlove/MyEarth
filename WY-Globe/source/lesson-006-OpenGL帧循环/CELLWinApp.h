#pragma once
#include "CELLApp.h"
#include <windows.h>
#include <tchar.h>
#include "CELLPlatform.hpp"
#include "CELLGLContext.hpp"

namespace CELL
{
	class CELLWinApp : public CELLApp 
	{
	public:
		HWND _hWnd;
		CELLGLContext _context;
	public:
		CELLWinApp::CELLWinApp()
		{
			_hWnd = 0;
		}

	public:
		bool createWindow(int width, int heigth, INSTANCE hWnd)
		{
			//1. ע�ᴰ��
			WNDCLASSEXW wcex;

			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = WndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = hWnd;
			wcex.hIcon = 0;
			wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = 0;
			wcex.lpszClassName = _T("CELL.BigMap");
			wcex.hIconSm = 0;

			RegisterClassExW(&wcex);

			//2. ��������
			_hWnd = CreateWindow(_T("CELL.BigMap"), _T("BigMap"), WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hWnd, nullptr);

			if (!_hWnd)
			{
				return false;
			}

			//3. ��ʾ����
			ShowWindow(_hWnd, SW_SHOW);
			UpdateWindow(_hWnd);

			HDISPLAY hDC = GetDC(_hWnd);
			if (!_context.init(_hWnd, hDC))
			{
				DestroyWindow(_hWnd);
				return false;
			}
			
			return true;
		}

		int main(int argc, char** argv)
		{
			MSG msg = { 0 };
#if 0
			// ����Ϣѭ��: 
			while (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
#else
			// ����Ϣѭ��: 
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				render();
			}
#endif
			_context.shutdown();
			return 0;
		}

		void render()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(1, 0, 0, 1);

			_context.swapBuffer();
		}
	protected:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: �ڴ˴����ʹ�� hdc ���κλ�ͼ����...
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
	};
}