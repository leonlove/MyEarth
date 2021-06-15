#pragma once
#include "CELLContext.h"

namespace CELL {

	class CELLFrame {

	public:
		CELLFrame(CELLContext& context):_context(context)
		{
		}

		virtual ~CELLFrame(){}

	public:
		virtual void update() = 0;

		virtual void onFrameStart() = 0;

		virtual void onFrameEnd() = 0;

	public:
		CELLContext  _context;
	};
}
