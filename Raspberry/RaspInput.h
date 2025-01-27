#pragma once
#include <string>

#include "glm/vec2.hpp"
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>

typedef unsigned long pthread_t;

enum MouseButtons
{
	Left = 0,
	Middle = 1,
	Right = 2
};


class RaspberryInput
{
public:
	RaspberryInput(const int& currentFrame);
	~RaspberryInput();

	bool GetMouseDown(MouseButtons button) const;
	glm::vec2 GetMousePosition() const;
	glm::vec2 GetMouseDelta() const;
	bool GetKeyDown(const int& code) const;

private:
	pthread_t mouseThread;
	pthread_t keyboardThread;

	bool* mouseDown;
	bool* keyDown;

	const int& currentFrame;
	
	bool isPolling{true};

	std::string keyboardLocation;

	glm::vec2 mousePosition{0, 0};
	glm::vec2 previousMousePosition{0, 0};
	int previousPollFrame{0};

	void FindKeyboardLocation();
	std::string FindActiveKeyboardEV();

	static void* ProcessMouseThread(void* arg);
	static void* ProcessKeyboardThread(void* arg);
};

