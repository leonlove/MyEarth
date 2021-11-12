#pragma once
#include "CELLApp.h"
#include <windows.h>
#include <tchar.h>
#include "CELLPlatform.hpp"
#include "CELLGLContext.hpp"
#include "CELLOpenGL.h"
#include "CELLFrameBigMap.h"
#include "CELLContext.h"

namespace CELL
{
	class CELLWinApp : public CELLApp 
	{
	public:
		HWND _hWnd;
		CELLGLContext		_contextgl;
		CELLOpenGL			_device;
		CELLContext			_context;
		CELLFrame*			_frame;
		bool				_threadRun;
	public:
		CELLWinApp::CELLWinApp()
		{
			_hWnd = 0;
			_frame = 0;
			_threadRun = true;
		}

	public:
		bool createWindow(int width, int heigth, INSTANCE hWnd)
		{
			//1. 注册窗口
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

			//2. 创建窗口
			_hWnd = CreateWindow(_T("CELL.BigMap"), _T("BigMap"), WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hWnd, this);

			if (!_hWnd)
			{
				return false;
			}

			//3. 显示窗口
			ShowWindow(_hWnd, SW_SHOW);
			UpdateWindow(_hWnd);

			//HDISPLAY hDC = GetDC(_hWnd);
			//if (!_contextgl.init(_hWnd, hDC))
			//{
			//	DestroyWindow(_hWnd);
			//	return false;
			//}
			
			return true;
		}

		virtual CELLFrame*  createFrame()
		{
			return  new CELLFrameBigMap(_context);
		}

		int main(int argc, char** argv)
		{
			_frame = createFrame();
			if (_frame != 0)
			{
				CELLThread::start();
#if 1
				MSG msg = { 0 };
				// 主消息循环: 
				while (msg.message != WM_QUIT)
				{
					if (GetMessage(&msg, nullptr, 0, 0))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
#else
				MSG msg = { 0 };
				// 主消息循环: 
				while (msg.message != WM_QUIT)
				{
					if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					render();
				}
				_contextgl.shutdown();
#endif
			}
			return 0;
		}

		void render()
		{
			if (_frame == 0)
			{
				return;
			}
			_frame->update();
			_frame->onFrameStart();
			_frame->onFrameEnd();

			_contextgl.swapBuffer();
		}

		LRESULT eventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_LBUTTONDOWN:
				_frame->onLButtonDown(LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_LBUTTONUP:
				_frame->onLButtonUp(LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_MOUSEMOVE:
				break;
			case WM_MOUSEWHEEL:
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
				_threadRun = false;
				CELLThread::join();
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			return S_OK;
		}

		/**
		*   创建完成通知函数
		*/
		virtual bool    onCreate()
		{
			HDISPLAY hDC = GetDC(_hWnd);
			if (!_contextgl.init(_hWnd, hDC))
			{
				DestroyWindow(_hWnd);
				return false;
			}
			return true;
		}

		/**
		*   线程执行函数
		*/
		virtual bool    onRun()
		{
			while (_threadRun)
			{
				render();
			}
			return true;
		}

		/**
		*   结束函数
		*/
		virtual bool    onDestroy()
		{
			_contextgl.shutdown();
			return true;
		}

	protected:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			if (WM_CREATE == message)
			{
				CREATESTRUCT* pSTRUCT = (CREATESTRUCT*)lParam;
				CELLWinApp*		pApp = (CELLWinApp*)pSTRUCT->lpCreateParams;
				#ifndef GWL_USERDATA
				#define GWL_USERDATA (-21)
				#endif
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pApp);
				//我觉得这句是没有太多必要的，除非WM_CREATE消息的同时，还有其他的消息
				//return pApp->eventProc(hWnd, message, wParam, lParam);
			}
			else
			{
				CELLWinApp* pApp = (CELLWinApp*)GetWindowLongPtr(hWnd, GWL_USERDATA);
				if (pApp)
				{
					return pApp->eventProc(hWnd, message, wParam, lParam);
				}
				else
				{
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			return S_OK;
		}
	};
}