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
	};
}


