#pragma once
#include "CELLApp.h"

namespace CELL {

	class CELLWinApp: public CELLApp
	{
	public:
		CELLWinApp();
		virtual ~CELLWinApp();

	public:
		virtual bool createwindow(HINSTANCE hInstance, int width, int height);

		virtual void main(int argc, char **argv);

	protected:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		HWND _hWnd;
	};
}