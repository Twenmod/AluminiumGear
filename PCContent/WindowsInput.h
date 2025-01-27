#pragma once
// 2 input classes
#include "IInput.h"
#include <GLFW/glfw3.h>

#include <iostream>

class WindowsKeyboard : public real::IKeyboard
{
public:
	WindowsKeyboard(GLFWwindow& window);
	bool GetKey(real::Key key) const override;
	void KeyCallBack() {}
private:
	GLFWwindow& window;
};


class WindowsMouse : public real::IMouse
{
public:
	WindowsMouse(GLFWwindow& window);

	bool GetButtonDown(real::MouseButtons button) const override;
	glm::vec2 GetPosition() const override;
	float GetScrollDelta() const override;
	void Tick() override;
	void ChangeMouseLockMode(real::MouseLockModes mouseLockMode) override;
private:
	GLFWwindow& window;

	static float scrollDelta;
	static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
};