<<<<<<< HEAD
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

		virtual bool	onLButtonDown(int x, int y) = 0;

		virtual bool	onLButtonUp(int x, int y) = 0;

		virtual bool	onMouseMove(int x, int y) = 0;

		virtual bool	onMouseWheel(int x, int y) = 0;

	public:
		CELLContext  _context;
	};
}
=======
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

		virtual bool	onLButtonDown(int x, int y) = 0;

		virtual bool	onLButtonUp(int x, int y) = 0;

		virtual bool	onMouseMove(int x, int y) = 0;

		virtual bool	onMouseWheel(int x, int y) = 0;

	public:
		CELLContext  _context;
	};
}
>>>>>>> 1894bd4604821c0a5ca59f8abc5b7c348fab1a2f
