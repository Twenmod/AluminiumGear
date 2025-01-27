#pragma once
// group these 2 togeter

#include <glm/vec2.hpp>
#ifdef WINDOWS
#include <GLFW/glfw3.h>
#endif

namespace real
{
	enum class MouseButtons
	{
		NONE,
		LEFT,
		RIGHT,
		MIDDLE
	};
	enum class MouseLockModes
	{
		UNLOCKED,
		HIDDEN,
		LOCKED,
		LOCKEDANDHIDDEN
	};

	class IMouse
	{

	public:
		virtual bool GetButtonDown(MouseButtons button) const = 0;
		virtual glm::vec2 GetPosition() const = 0;
		virtual float GetScrollDelta() const = 0;
		virtual void Tick() = 0;
		virtual void ChangeMouseLockMode(MouseLockModes mouseLockMode) = 0;
		virtual ~IMouse() = default;
		bool m_locked = false;
		bool m_hidden = false;
	};



	enum class Key
	{
		NONE,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		NUM_0,
		NUM_1,
		NUM_2,
		NUM_3,
		NUM_4,
		NUM_5,
		NUM_6,
		NUM_7,
		NUM_8,
		NUM_9,
		TAB,
		CAPS_LOCK,
		SHIFT_LEFT,
		CTRL_LEFT,
		ALT_LEFT,
		ESCAPE,
		RIGHT_SHIFT,
		ENTER,
		ARROW_UP,
		ARROW_RIGHT,
		ARROW_DOWN,
		ARROW_LEFT,
		SPACE
	};

	class IKeyboard
	{
	public:
		virtual bool GetKey(Key key) const = 0;
		virtual ~IKeyboard() = default;
	};
}