#pragma once
#include "precomp.h"
#include "Input.h"
#include "IInput.h"
#include "WindowsInput.h"
#include <GLFW/glfw3.h>

#include "ImGui/imgui_impl_pi.h"

#include <iostream>

WindowsMouse::WindowsMouse(GLFWwindow& window) : window(window)
{
	glfwSetScrollCallback(&window, ScrollCallback);
}

bool WindowsMouse::GetButtonDown(real::MouseButtons button) const
{
	switch (button)
	{
	case real::MouseButtons::LEFT: return glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_LEFT);
	case real::MouseButtons::RIGHT: return glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_RIGHT);
	case real::MouseButtons::MIDDLE: return glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_MIDDLE);
	default:std::cout << "ERROR::INPUT::WINDOWS Mouse button not supported: " << static_cast<int>(button) << std::endl; return false;
	}
}

glm::vec2 WindowsMouse::GetPosition() const
{
	double xPosition, yPosition;
	glfwGetCursorPos(&window, &xPosition, &yPosition);
	return glm::vec2{ xPosition, yPosition };
}

void WindowsMouse::ChangeMouseLockMode(real::MouseLockModes _mouseLockMode)
{
	switch (_mouseLockMode)
	{
		case real::MouseLockModes::UNLOCKED:
			glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_locked = false;
			m_hidden = false;
			break;
		case real::MouseLockModes::HIDDEN:
			glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			m_hidden = true;
			m_locked = false;
			break;
		case real::MouseLockModes::LOCKED:
			glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // We dont use glfw's locked for consistency with pi
			m_hidden = false;
			m_locked = true;
			break;
		case real::MouseLockModes::LOCKEDANDHIDDEN:
			glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // We dont use glfw's locked for consistency with pi
			m_hidden = true;
			m_locked = true;
			break;

	}
};

void WindowsMouse::Tick()
{
	if (m_locked)
	{
		int sizeX, sizeY;
		glfwGetWindowSize(&window, &sizeX, &sizeY);
		glfwSetCursorPos(&window, sizeX / 2, sizeY / 2);
	}
}


float WindowsMouse::GetScrollDelta() const
{
	return scrollDelta;
}

float WindowsMouse::scrollDelta{ 0.0f };

void WindowsMouse::ScrollCallback(GLFWwindow* /*window*/, double /*xOffset*/, double yOffset)
{
	scrollDelta = static_cast<float>(yOffset);
}

real::Key ConvertKey(int key)
{
	switch (key)
	{
		case GLFW_KEY_A: return real::Key::A;
		case GLFW_KEY_B: return real::Key::B;
		case GLFW_KEY_C: return real::Key::C;
		case GLFW_KEY_D: return real::Key::D;
		case GLFW_KEY_E: return real::Key::E;
		case GLFW_KEY_F: return real::Key::F;
		case GLFW_KEY_G: return real::Key::G;
		case GLFW_KEY_H: return real::Key::H;
		case GLFW_KEY_I: return real::Key::I;
		case GLFW_KEY_J: return real::Key::J;
		case GLFW_KEY_K: return real::Key::K;
		case GLFW_KEY_L: return real::Key::L;
		case GLFW_KEY_M: return real::Key::M;
		case GLFW_KEY_N: return real::Key::N;
		case GLFW_KEY_O: return real::Key::O;
		case GLFW_KEY_P: return real::Key::P;
		case GLFW_KEY_Q: return real::Key::Q;
		case GLFW_KEY_R: return real::Key::R;
		case GLFW_KEY_S: return real::Key::S;
		case GLFW_KEY_T: return real::Key::T;
		case GLFW_KEY_U: return real::Key::U;
		case GLFW_KEY_V: return real::Key::V;
		case GLFW_KEY_W: return real::Key::W;
		case GLFW_KEY_X: return real::Key::X;
		case GLFW_KEY_Y: return real::Key::Y;
		case GLFW_KEY_Z: return real::Key::Z;
		case GLFW_KEY_0: return  real::Key::NUM_0;
		case GLFW_KEY_1: return  real::Key::NUM_1;
		case GLFW_KEY_2: return  real::Key::NUM_2;
		case GLFW_KEY_3: return  real::Key::NUM_3;
		case GLFW_KEY_4: return  real::Key::NUM_4;
		case GLFW_KEY_5: return  real::Key::NUM_5;
		case GLFW_KEY_6: return  real::Key::NUM_6;
		case GLFW_KEY_7: return  real::Key::NUM_7;
		case GLFW_KEY_8: return  real::Key::NUM_8;
		case GLFW_KEY_9: return  real::Key::NUM_9;
		case GLFW_KEY_TAB: return  real::Key::TAB;
		case GLFW_KEY_CAPS_LOCK: return  real::Key::CAPS_LOCK;
		case GLFW_KEY_LEFT_SHIFT: return  real::Key::SHIFT_LEFT;
		case GLFW_KEY_LEFT_CONTROL: return  real::Key::CTRL_LEFT;
		case GLFW_KEY_LEFT_ALT: return  real::Key::ALT_LEFT;
		case GLFW_KEY_ESCAPE: return  real::Key::ESCAPE;
		case GLFW_KEY_RIGHT_SHIFT: return  real::Key::RIGHT_SHIFT;
		case GLFW_KEY_ENTER: return  real::Key::ENTER;
		case GLFW_KEY_UP: return  real::Key::ARROW_UP;
		case GLFW_KEY_RIGHT: return  real::Key::ARROW_RIGHT;
		case GLFW_KEY_DOWN: return  real::Key::ARROW_DOWN;
		case GLFW_KEY_LEFT: return  real::Key::ARROW_LEFT;
		case GLFW_KEY_SPACE: return  real::Key::SPACE;
		default: return real::Key::NONE;
	}
}

int KeyToUnicode(real::Key key)
{
	switch (key)
	{
		case real::Key::A: return 'a';
		case real::Key::B: return 'b';
		case real::Key::C: return 'c';
		case real::Key::D: return 'd';
		case real::Key::E: return 'e';
		case real::Key::F: return 'f';
		case real::Key::G: return 'g';
		case real::Key::H: return 'h';
		case real::Key::I: return 'i';
		case real::Key::J: return 'j';
		case real::Key::K: return 'k';
		case real::Key::L: return 'l';
		case real::Key::M: return 'm';
		case real::Key::N: return 'n';
		case real::Key::O: return 'o';
		case real::Key::P: return 'p';
		case real::Key::Q: return 'q';
		case real::Key::R: return 'r';
		case real::Key::S: return 's';
		case real::Key::T: return 't';
		case real::Key::U: return 'u';
		case real::Key::V: return 'v';
		case real::Key::W: return 'w';
		case real::Key::X: return 'x';
		case real::Key::Y: return 'y';
		case real::Key::Z: return 'z';
		case real::Key::NUM_0: return '0';
		case real::Key::NUM_1: return '1';
		case real::Key::NUM_2: return '2';
		case real::Key::NUM_3: return '3';
		case real::Key::NUM_4: return '4';
		case real::Key::NUM_5: return '5';
		case real::Key::NUM_6: return '6';
		case real::Key::NUM_7: return '7';
		case real::Key::NUM_8: return '8';
		case real::Key::NUM_9: return '9';
		case real::Key::SPACE: return ' ';
		default: return -1;  // Non-printable keys or undefined keys
	}
}

void key_callback(GLFWwindow*, int key, int scancode, int action, int)
{
	real::Key iKey = ConvertKey(key);

	bool down = action == GLFW_PRESS ? true : false;

	ImGui_ImplPi_KeyCallback(iKey, scancode, down);

	if (down)
	{
		unsigned int c = KeyToUnicode(iKey);
		if (c != -1)
			ImGui_ImplPi_CharCallback(c);
	}
}
WindowsKeyboard::WindowsKeyboard(GLFWwindow& window) : window(window)
{
	glfwSetKeyCallback(&window, key_callback);
}





bool WindowsKeyboard::GetKey(real::Key key) const
{
	switch (key) {
	case real::Key::A: return glfwGetKey(&window, GLFW_KEY_A);
	case real::Key::B: return glfwGetKey(&window, GLFW_KEY_B);
	case real::Key::C: return glfwGetKey(&window, GLFW_KEY_C);
	case real::Key::D: return glfwGetKey(&window, GLFW_KEY_D);
	case real::Key::E: return glfwGetKey(&window, GLFW_KEY_E);
	case real::Key::F: return glfwGetKey(&window, GLFW_KEY_F);
	case real::Key::G: return glfwGetKey(&window, GLFW_KEY_G);
	case real::Key::H: return glfwGetKey(&window, GLFW_KEY_H);
	case real::Key::I: return glfwGetKey(&window, GLFW_KEY_I);
	case real::Key::J: return glfwGetKey(&window, GLFW_KEY_J);
	case real::Key::K: return glfwGetKey(&window, GLFW_KEY_K);
	case real::Key::L: return glfwGetKey(&window, GLFW_KEY_L);
	case real::Key::M: return glfwGetKey(&window, GLFW_KEY_M);
	case real::Key::N: return glfwGetKey(&window, GLFW_KEY_N);
	case real::Key::O: return glfwGetKey(&window, GLFW_KEY_O);
	case real::Key::P: return glfwGetKey(&window, GLFW_KEY_P);
	case real::Key::Q: return glfwGetKey(&window, GLFW_KEY_Q);
	case real::Key::R: return glfwGetKey(&window, GLFW_KEY_R);
	case real::Key::S: return glfwGetKey(&window, GLFW_KEY_S);
	case real::Key::T: return glfwGetKey(&window, GLFW_KEY_T);
	case real::Key::U: return glfwGetKey(&window, GLFW_KEY_U);
	case real::Key::V: return glfwGetKey(&window, GLFW_KEY_V);
	case real::Key::W: return glfwGetKey(&window, GLFW_KEY_W);
	case real::Key::X: return glfwGetKey(&window, GLFW_KEY_X);
	case real::Key::Y: return glfwGetKey(&window, GLFW_KEY_Y);
	case real::Key::Z: return glfwGetKey(&window, GLFW_KEY_Z);
	case real::Key::NUM_0: return glfwGetKey(&window, GLFW_KEY_0);
	case real::Key::NUM_1: return glfwGetKey(&window, GLFW_KEY_1);
	case real::Key::NUM_2: return glfwGetKey(&window, GLFW_KEY_2);
	case real::Key::NUM_3: return glfwGetKey(&window, GLFW_KEY_3);
	case real::Key::NUM_4: return glfwGetKey(&window, GLFW_KEY_4);
	case real::Key::NUM_5: return glfwGetKey(&window, GLFW_KEY_5);
	case real::Key::NUM_6: return glfwGetKey(&window, GLFW_KEY_6);
	case real::Key::NUM_7: return glfwGetKey(&window, GLFW_KEY_7);
	case real::Key::NUM_8: return glfwGetKey(&window, GLFW_KEY_8);
	case real::Key::NUM_9: return glfwGetKey(&window, GLFW_KEY_9);
	case real::Key::TAB: return glfwGetKey(&window, GLFW_KEY_TAB);
	case real::Key::CAPS_LOCK: return glfwGetKey(&window, GLFW_KEY_CAPS_LOCK);
	case real::Key::SHIFT_LEFT: return glfwGetKey(&window, GLFW_KEY_LEFT_SHIFT);
	case real::Key::CTRL_LEFT: return glfwGetKey(&window, GLFW_KEY_LEFT_CONTROL);
	case real::Key::ALT_LEFT:return glfwGetKey(&window, GLFW_KEY_LEFT_ALT);
	case real::Key::ESCAPE: return glfwGetKey(&window, GLFW_KEY_ESCAPE);
	case real::Key::RIGHT_SHIFT: return glfwGetKey(&window, GLFW_KEY_RIGHT_SHIFT);
	case real::Key::ENTER: return glfwGetKey(&window, GLFW_KEY_ENTER);
	case real::Key::ARROW_UP: return glfwGetKey(&window, GLFW_KEY_UP);
	case real::Key::ARROW_RIGHT: return glfwGetKey(&window, GLFW_KEY_RIGHT);
	case real::Key::ARROW_DOWN: return glfwGetKey(&window, GLFW_KEY_DOWN);
	case real::Key::ARROW_LEFT: return glfwGetKey(&window, GLFW_KEY_LEFT);
	case real::Key::SPACE: return glfwGetKey(&window, GLFW_KEY_SPACE);
	default: std::cout << "ERROR::INPUT::WINDOWS Keycode not supported: " << static_cast<int>(key) << std::endl; return false;
	}
}
