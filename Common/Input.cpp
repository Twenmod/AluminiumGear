#include "precomp.h"
#include "IInput.h"
#include "Input.h"

#include "IInputEvent.h"

#include "ImGui/imgui_impl_pi.h"

real::InputManager::InputManager(const IKeyboard* const keyboard, IMouse* const mouse) :
	m_mouse(mouse),
	m_keyboard(keyboard)
{

	for (int i = 0; i < INPUT_ACTION_AMOUNT; i++)
	{
		m_inputActions[i].action = E_INPUTACTIONS::NONE;
		m_inputActions[i].value = 0;
	}

	//Define input actions
	//TODO: This should not be done in engine code

	InputAction iAction;
	iAction.action = VERTICAL_MOVE;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::W;
	iAction.negative.key = Key::S;
	m_inputActions[VERTICAL_MOVE] = iAction;

	iAction.action = HORIZONTAL_MOVE;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::D;
	iAction.negative.key = Key::A;
	m_inputActions[HORIZONTAL_MOVE] = iAction;

	iAction.action = SPRINT;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::SHIFT_LEFT;
	iAction.negative.key = Key::NONE;
	m_inputActions[SPRINT] = iAction;

	iAction.action = JUMP_CROUCH;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::SPACE;
	iAction.negative.key = Key::CTRL_LEFT;
	m_inputActions[JUMP_CROUCH] = iAction;

	iAction.action = WEAPON_USE;
	iAction.isMouseAction = true;
	iAction.positive.mousebutton = MouseButtons::LEFT;
	iAction.negative.mousebutton = MouseButtons::RIGHT;
	m_inputActions[WEAPON_USE] = iAction;

	iAction.action = ITEM_USE;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::SPACE;
	iAction.negative.key = Key::ALT_LEFT;
	m_inputActions[ITEM_USE] = iAction;

	iAction.action = INTERACT;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::E;
	iAction.negative.key = Key::NONE;
	m_inputActions[INTERACT] = iAction;

	iAction.action = PAUSE;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::ESCAPE;
	iAction.negative.key = Key::NONE;
	m_inputActions[PAUSE] = iAction;

	iAction.action = UI_HORIZONTAL;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::ARROW_LEFT;
	iAction.negative.key = Key::ARROW_RIGHT;
	m_inputActions[UI_HORIZONTAL] = iAction;

	iAction.action = UI_VERTICAL;
	iAction.isMouseAction = false;
	iAction.positive.key = Key::ARROW_UP;
	iAction.negative.key = Key::ARROW_DOWN;
	m_inputActions[UI_VERTICAL] = iAction;


	m_mousePosition = mouse->GetPosition();
}

real::InputManager::~InputManager()
{
	delete(m_mouse);
	delete(m_keyboard);
}

void real::InputManager::Tick()
{
	//Mouse
	if (m_mouse->m_locked) m_mouseDelta = m_mousePosition - m_mouse->GetPosition();
	else m_mouseDelta = glm::vec2(0);

	if (m_ignoreMouseOnce)
	{
		m_mouseDelta = glm::vec2(0);
		m_ignoreMouseOnce = false;
	}

	//Actions
	InputAction triggeredActions[INPUT_ACTION_AMOUNT];
	unsigned int triggeredActionsIterator = 0;
	for (int i = 0; i < INPUT_ACTION_AMOUNT; i++)
	{
		InputAction& action = m_inputActions[i];
		if (action.action != E_INPUTACTIONS::NONE)
		{
			float value = 0;
			if (action.isMouseAction)
			{
				if (action.positive.mousebutton != MouseButtons::NONE && m_mouse->GetButtonDown(action.positive.mousebutton)) value += 1;
				if (action.negative.mousebutton != MouseButtons::NONE && m_mouse->GetButtonDown(action.negative.mousebutton)) value -= 1;
			}
			else
			{
				if (action.positive.key != Key::NONE && m_keyboard->GetKey(action.positive.key)) value += 1;
				if (action.negative.key != Key::NONE && m_keyboard->GetKey(action.negative.key)) value -= 1;
			}
			if (value != action.value) // value changed
			{
				action.value = value;
				triggeredActions[triggeredActionsIterator++] = action;
			}
		}
	}

	for (unsigned int i = 0; i < m_subscribers.size(); i++)
	{
		for (unsigned int j = 0; j < triggeredActionsIterator; j++)
		{
			m_subscribers[i]->OnInputAction(triggeredActions[j]);
		}
	}

	m_mouse->Tick();
	m_mousePosition = m_mouse->GetPosition();
}

void real::InputManager::TickImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT);
	ImGui_ImplPi_CursorPosCallback(m_mousePosition.x, m_mousePosition.y);
	ImGui_ImplPi_MouseButtonCallback(0, m_mouse->GetButtonDown(real::MouseButtons::LEFT));
	ImGui_ImplPi_MouseButtonCallback(1, m_mouse->GetButtonDown(real::MouseButtons::RIGHT));
	ImGui_ImplPi_ScrollCallback(0, m_mouse->GetScrollDelta());
}


const real::IKeyboard& real::InputManager::GetKeyboard() const
{
	return *m_keyboard;
}

real::IMouse& real::InputManager::GetMouse() const
{
	return *m_mouse;
}
