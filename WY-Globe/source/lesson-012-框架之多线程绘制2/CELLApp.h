<<<<<<< HEAD
#pragma once
#include "CELLPlatform.hpp"
#include "CELLThread.hpp"

namespace CELL
{
	class CELLApp : public CELLThread
	{
	public:
		//�������ں���
		virtual bool createWindow(int width, int heigth, INSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
		
		/**
		*   �������֪ͨ����
		*/
		virtual bool    onCreate() = 0;

		/**
		*   �߳�ִ�к���
		*/
		virtual bool    onRun() = 0;

		/**
		*   ��������
		*/
		virtual bool    onDestroy() = 0;

	};
}
=======
#pragma once
#include "CELLPlatform.hpp"
#include "CELLThread.hpp"

namespace CELL
{
	class CELLApp : public CELLThread
	{
	public:
		//�������ں���
		virtual bool createWindow(int width, int heigth, INSTANCE hWnd) = 0;

		virtual int main(int argc, char** argv) = 0;
		
		/**
		*   �������֪ͨ����
		*/
		virtual bool    onCreate() = 0;

		/**
		*   �߳�ִ�к���
		*/
		virtual bool    onRun() = 0;

		/**
		*   ��������
		*/
		virtual bool    onDestroy() = 0;

	};
}
>>>>>>> 1894bd4604821c0a5ca59f8abc5b7c348fab1a2f
