#pragma once
#include "CELLApp.h"
#include "CELLPlatform.hpp"
#include "CELLGLContext.hpp"

namespace CELL {

	class CELLWinApp: public CELLApp
	{
	public:
		CELLWinApp();
		virtual ~CELLWinApp();

	public:
		virtual bool createwindow(INSTANCE hInstance, int width, int height);

		virtual void main(int argc, char **argv);

		void render();

		LRESULT eventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		static LRESULT CALLBACK WndProc(HWINDOW hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		HWINDOW			_hWnd;
		CELLGLContext	_contextGL;
	};
}