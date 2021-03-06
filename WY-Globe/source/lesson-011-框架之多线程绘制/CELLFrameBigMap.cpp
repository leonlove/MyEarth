#include "CELLFrameBigMap.h"
#include "CELLContext.h"


namespace CELL {

	CELLFrameBigMap::CELLFrameBigMap(CELLContext& context)
		:CELLFrame(context)
	{
	}


	CELLFrameBigMap::~CELLFrameBigMap()
	{
	}

	void CELLFrameBigMap::update()
	{

	}

	void CELLFrameBigMap::onFrameStart()
	{
		_context._device->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_context._device->clearColor(1, 0, 0, 1);
	}

	void CELLFrameBigMap::onFrameEnd()
	{

	}

	bool CELLFrameBigMap::onLButtonDown(int x, int y)
	{
		return true;
	}

	bool CELLFrameBigMap::onLButtonUp(int x, int y)
	{
		return true;
	}

	bool CELLFrameBigMap::onMouseMove(int x, int y)
	{
		return true;
	}

	bool CELLFrameBigMap::onMouseWheel(int x, int y)
	{
		return true;
	}
}

