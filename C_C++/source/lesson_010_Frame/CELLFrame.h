#pragma once
#include "CELLContext.h"
#include "CELLInput.h"

namespace CELL {
	class CELLFrame : public CELLInput {
	public:
		virtual void update(CELLContext& context) = 0;

		virtual void startframe(CELLContext& context) = 0;

		virtual void endframe(CELLContext& context) = 0;

		//左键按下
		virtual void onLButtonDown(int x, int y) = 0;

		//右键按下
		virtual void onRButtonDown(int x, int y) = 0;

		//鼠标滚轮
		virtual void onMouseMove(int x, int y) = 0;
	};
}
