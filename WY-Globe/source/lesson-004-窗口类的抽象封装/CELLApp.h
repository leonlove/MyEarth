#pragma once

namespace CELL
{
	class CELLApp
	{
	public:
		//�������ں���
		virtual bool createWindow(int width, int heigth, HINSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
	};
}
