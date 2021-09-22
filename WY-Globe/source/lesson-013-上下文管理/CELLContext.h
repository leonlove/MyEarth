#pragma once
#include    "CELLMath.hpp"
#include	"CELLOpenGL.h"

namespace CELL
{
	//上下文资源管理
	class CELLContext {
	public:
		CELLOpenGL*		_device;

		// 鼠标位置
		int				_mouseX;
		int				_mouseY;

		// 窗口的大小
		int				_width;
		int				_height;


		matrix4r		_screenPrj;
	};
}