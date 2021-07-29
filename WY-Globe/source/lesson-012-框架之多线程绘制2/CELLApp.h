#pragma once
#include "CELLPlatform.hpp"
#include "CELLThread.hpp"

namespace CELL
{
	class CELLApp : public CELLThread
	{
	public:
		//创建窗口函数
		virtual bool createWindow(int width, int heigth, INSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
		
		/**
		*   创建完成通知函数
		*/
		virtual bool    onCreate() = 0;

		/**
		*   线程执行函数
		*/
		virtual bool    onRun() = 0;

		/**
		*   结束函数
		*/
		virtual bool    onDestroy() = 0;

	};
}
