#pragma once

namespace CELL
{
	class CELLInput
	{
	public:
		virtual bool	onLButtonDown(int x, int y) = 0;

		virtual bool	onLButtonUp(int x, int y) = 0;

		virtual bool	onMouseMove(int x, int y) = 0;

		virtual bool	onMouseWheel(int x, int y) = 0;
	};
}