<<<<<<< HEAD
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


=======
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


>>>>>>> 1894bd4604821c0a5ca59f8abc5b7c348fab1a2f
