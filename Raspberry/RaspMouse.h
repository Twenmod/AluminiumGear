#pragma once
#include "IInput.h"

#include <X11/Xlib.h>

class RaspMouse : public real::IMouse
{
public:
	RaspMouse(Display& display, Window& window);

	bool GetButtonDown(real::MouseButtons button) const override;
	glm::vec2 GetPosition() const override;
	float GetScrollDelta() const override;
	void Tick() override;
	void ChangeMouseLockMode(real::MouseLockModes mouseLockMode) override;
private:
	void SetCursorDisplayMode(bool hidden = true);
	Display& display ;
	Window& window ; 
};

