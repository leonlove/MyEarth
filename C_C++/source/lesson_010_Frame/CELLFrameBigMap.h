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

		//�������
		virtual void onLButtonDown(int x, int y) {

		}

		//�Ҽ�����
		virtual void onRButtonDown(int x, int y) {

		}

		//������
		virtual void onMouseMove(int x, int y) {

		}
	};
}


