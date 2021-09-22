#pragma once
#include "CELLFrame.h"
#include "CELLContext.h"
#include "CELLOpenGL.h"

namespace CELL
{
	class CELLFrameBigMap : public CELLFrame
	{
	public:
		CELLFrameBigMap();
		virtual ~CELLFrameBigMap();


		virtual void update(CELLContext& context) {
			context._device->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			context._device->clearColor(1, 0, 0, 1);
		}

		virtual void startframe(CELLContext& context) {

		}

		virtual void endframe(CELLContext& context) {

		}

		//左键按下
		virtual void onLButtonDown(int x, int y) {

		}

		//右键按下
		virtual void onRButtonDown(int x, int y) {

		}

		//鼠标滚轮
		virtual void onMouseMove(int x, int y) {

		}
	};
}


