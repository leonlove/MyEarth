#pragma once

namespace CELL {
	class CELLInput {
	public:
		virtual void onLButtonDown(int x, int y) = 0;

		virtual void onLButtonUp(int x, int y) = 0;
	};
}