#pragma once
#include "CELLPlatform.hpp"

namespace CELL
{
	class CELLApp
	{
	public:
		//创建窗口函数
		virtual bool createWindow(int width, int heigth, INSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
	};
}
