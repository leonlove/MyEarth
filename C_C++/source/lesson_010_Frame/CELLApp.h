#pragma once
#include "CELLFrame.h"

namespace CELL
{
	class CELLApp
	{
	public:
		virtual bool createwindow(HINSTANCE hInstance, int width, int height) = 0;

		virtual CELLFrame* createframe() = 0;

		virtual void main(int argc, char **argv) = 0;
	};

}
