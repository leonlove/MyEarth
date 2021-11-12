#pragma once
#include "CELLContext.h"
#include "CELLInput.h"

namespace CELL {

	class CELLFrame : public CELLInput
	{

	public:
		CELLFrame(CELLContext& context):_context(context)
		{
		}

		virtual ~CELLFrame(){}

	public:
		virtual void update() = 0;

		virtual void onFrameStart() = 0;

		virtual void onFrameEnd() = 0;

		virtual void onLButtonDown(int x, int y) = 0;

		virtual void onLButtonUp(int x, int y) = 0;

	public:
		CELLContext  _context;
	};
}
