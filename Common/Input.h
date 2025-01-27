#pragma once

namespace real
{
	class IMouse;
	class IKeyboard;
	class IInputEvent;
	enum class Key;
	enum class MouseButtons;

	struct InputAction
	{
		E_INPUTACTIONS action;
		bool isMouseAction;
		union
		{
			Key key;
			MouseButtons mousebutton;
		} positive, negative;
		float value;
	};


	class InputManager
	{
	public:
		InputManager(const IKeyboard* const keyboard, IMouse* const mouse);
		~InputManager();
		void Tick();
		void TickImGui();
		void Subscribe(IInputEvent* subscriber) { m_subscribers.push_back(subscriber); };
		void UnSubscribe(IInputEvent* subscriberToRemove)
		{
			auto found = std::find(m_subscribers.begin(), m_subscribers.end(), subscriberToRemove);
			if (found != m_subscribers.end()) m_subscribers.erase(found);
		};
		glm::vec2 GetMouseDelta() { return m_mouseDelta; }
		void IgnoreNextFrameMouseMovement() { m_ignoreMouseOnce = true; }
		const IKeyboard& GetKeyboard() const;
		IMouse& GetMouse() const;

	private:
		IMouse* const m_mouse;
		bool m_ignoreMouseOnce{false};
		const IKeyboard* const m_keyboard;
		std::vector<IInputEvent*> m_subscribers;
		InputAction m_inputActions[INPUT_ACTION_AMOUNT];
		const float m_MOUSESENSITIVITY = 0.1f;
		glm::vec2 m_mousePosition{ glm::vec2(0) };
		glm::vec2 m_mouseDelta{ glm::vec2(0) };
	};
}