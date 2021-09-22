#pragma once
#include    "CELLMath.hpp"
#include	"CELLOpenGL.h"

namespace CELL
{
	//��������Դ����
	class CELLContext {
	public:
		CELLOpenGL*		_device;

		// ���λ��
		int				_mouseX;
		int				_mouseY;

		// ���ڵĴ�С
		int				_width;
		int				_height;


		matrix4r		_screenPrj;
	};
}