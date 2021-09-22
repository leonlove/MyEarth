#pragma once
#include "CELLFrame.h"
#include "CELLContext.h"

namespace CELL {
	class CELLFrameBigMap : public CELLFrame
	{
	public:
		CELLFrameBigMap(CELLContext& context);
		virtual ~CELLFrameBigMap();

		virtual void update();

		virtual void onFrameStart();

		virtual void onFrameEnd();

		virtual bool	onLButtonDown(int x, int y);

		virtual bool	onLButtonUp(int x, int y);

		virtual bool	onMouseMove(int x, int y);

		virtual bool	onMouseWheel(int x, int y);
	};
}


