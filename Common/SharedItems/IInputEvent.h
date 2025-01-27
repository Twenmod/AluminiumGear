#pragma once
#include "Input.h"
namespace real
{
	class IInputEvent
	{
	public:
		virtual void OnInputAction(InputAction action) = 0;
	private:
	};
}