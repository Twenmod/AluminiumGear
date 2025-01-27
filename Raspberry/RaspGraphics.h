#pragma once
#include "IGraphics.h"

class XWindow;
struct EGLState;

class Game;

class RaspGraphics : public IGraphics
{
public:
	RaspGraphics();
	~RaspGraphics() {};

	void Quit() override;

	void SwapBuffer() override; // 

	XWindow& Window() const;
	void SetFullScreen(bool fullScreen) override;

	Game* game;
private:
	XWindow* window;
};

