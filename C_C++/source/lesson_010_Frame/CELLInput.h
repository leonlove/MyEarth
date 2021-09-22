#pragma once
namespace CELL {
	class CELLInput {
	public:
		virtual void onLButtonDown(int x, int y) = 0;

		virtual void onRButtonDown(int x, int y) = 0;

		virtual void onMouseMove(int x, int y) = 0;
	};
}