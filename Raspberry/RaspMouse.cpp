#include "precomp.h"
#include "RaspMouse.h"

RaspMouse::RaspMouse(Display& display, Window& window) :
	display(display),
	window(window)
{
}

bool RaspMouse::GetButtonDown(real::MouseButtons button) const
{
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	uint mask_return;
	Window window_returned;
	XQueryPointer(
		&display,
		window,
		&window_returned,
		&window_returned,

		&root_x,
		&root_y,
		&win_x,
		&win_y,
		&mask_return);

	switch (button) 
	{
		case real::MouseButtons::LEFT:
		{
			return mask_return & Button1MotionMask;
		}
		case real::MouseButtons::RIGHT:
		{
			return mask_return & Button3MotionMask;
		}
		case real::MouseButtons::MIDDLE:
		{
			return mask_return & Button2MotionMask;
		}
		default:
		{
			return false;
		}
	}
}

glm::vec2 RaspMouse::GetPosition() const
{
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	uint mask_return;
	Window window_returned;
	XQueryPointer(
		&display,
		window,
		&window_returned,
		&window_returned,

		&root_x,
		&root_y,
		&win_x,
		&win_y,
		&mask_return);

	return glm::vec2{win_x, win_y};
}

float RaspMouse::GetScrollDelta() const
{
	return 0;
}


void RaspMouse::ChangeMouseLockMode(real::MouseLockModes mouseLockMode)
{
	switch (mouseLockMode)
	{
		case real::MouseLockModes::UNLOCKED:
			SetCursorDisplayMode(false);
			m_locked = false;
			m_hidden = false;
			break;
		case real::MouseLockModes::HIDDEN:
			SetCursorDisplayMode(true);
			m_hidden = true;
			m_locked = false;
			break;
		case real::MouseLockModes::LOCKED:
			SetCursorDisplayMode(false);
			m_hidden = false;
			m_locked = true;
			break;
		case real::MouseLockModes::LOCKEDANDHIDDEN:
			SetCursorDisplayMode(true);
			m_hidden = true;
			m_locked = true;
			break;

	}
};

void RaspMouse::Tick()
{
	if (m_locked)
	{
		int x, y;
		x = 0;
		y = 0;
		int sizeX, sizeY;
		XWindowAttributes attr;
		XGetWindowAttributes(&display, window, &attr);
		sizeX = attr.width;
		sizeY = attr.height;
		XWarpPointer(&display, None, window, 0, 0, 0, 0, x + sizeX / 2, y + sizeY / 2);

	}
}

void RaspMouse::SetCursorDisplayMode(bool hidden)
{
	if (hidden)
	{
		Cursor invisibleCursor;
		Pixmap bitmapNoData;
		XColor black;
		static char noData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		black.red = black.green = black.blue = 0;
		bitmapNoData = XCreateBitmapFromData(&display, window, noData, 8, 8);
		invisibleCursor = XCreatePixmapCursor(&display, bitmapNoData, bitmapNoData, &black, &black, 0, 0);
		XDefineCursor(&display, window, invisibleCursor);
		XFreeCursor(&display, invisibleCursor);
		XFreePixmap(&display, bitmapNoData);
	}
	else
	{
		XUndefineCursor(&display, window);
	}
}