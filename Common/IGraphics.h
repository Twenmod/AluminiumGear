// a simple interface class its basically just simple access to graphics
#pragma once
class IGraphics
{
public:
	virtual ~IGraphics() = default;
	virtual void SwapBuffer() = 0;
	virtual void Quit() = 0;
	virtual void SetFullScreen(bool fullScreen) = 0;
};

