#pragma once
#include "CELLPlatform.hpp"

namespace CELL
{
	class CELLApp
	{
	public:
		//�������ں���
		virtual bool createWindow(int width, int heigth, INSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
	};
}
