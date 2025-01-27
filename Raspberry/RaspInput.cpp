#include "RaspInput.h"

#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <linux/input.h>

RaspberryInput::RaspberryInput(const int& currentFrame) : mouseDown{new bool[3]{false}}, keyDown{new bool[256]{false}}, currentFrame(currentFrame)
{
	FindKeyboardLocation();
	pthread_create(&mouseThread, nullptr, &ProcessMouseThread, this);
	pthread_create(&keyboardThread, nullptr, &ProcessKeyboardThread, this);

}

RaspberryInput::~RaspberryInput()
{

}

bool RaspberryInput::GetMouseDown(MouseButton button) const
{
	return mouseDown[button];
}

glm::vec2 RaspberryInput::GetMousePosition() const
{
	glm::vec2 clampedPos;

	clampedPos.x = mousePosition.x < 0 ? 0 : mousePosition.x;
	clampedPos.x = mousePosition.x > 1920 ? 1920 : mousePosition.x;
	clampedPos.y = mousePosition.y < 0 ? 0 : mousePosition.y;
	clampedPos.y = mousePosition.y > 1080 ? 1080 : mousePosition.y;

	return clampedPos;
}
y
glm::vec2 RaspberryInput::GetMouseDelta() const
{
	if(currentFrame - previousPollFrame > 1)
	{
		return glm::vec2{0};
	}

	return mousePosition - previousMousePosition;
}

bool RaspberryInput::GetKeyDown(const int& code) const
{
	return keyDown[code];
}

void RaspberryInput::FindKeyboardLocation()
{
	std::string ev {FindActiveKeyboardEV()};
	keyboardLocation = "/dev/input/" + ev;
}

std::string RaspberryInput::FindActiveKeyboardEV()
{
	std::ifstream devicesFile;
	devicesFile.open("/proc/bus/input/devices");

	std::stringstream devicesStream;

	devicesStream << devicesFile.rdbuf();
	devicesFile.close();

	std::string devices = devicesStream.str();

	// The index in the file for our current new line position.
	std::size_t newLinePos{0};

	// The EV value for the current device.
	std::string ev{};

	// Moves every new line in devices file.
	do
	{
		// Checks for the Handlers field.
		// Always caches it, so we can use it if we find the correct device.
		if(devices.substr(newLinePos + 1, 11) == "H: Handlers")
		{
			// Index of the end of the line.
			std::size_t nextNewLine{devices.find('\n', newLinePos + 1) - 1};

			// The contents of the EV line.
			std::string line{devices.substr(newLinePos + 1, nextNewLine - (newLinePos + 1))};

			// The index of the last space, so we can get the last value.
			std::size_t lastSpace{line.rfind(' ')};

			// The very last value, which is the event.
			ev = line.substr(lastSpace + 1, line.length() - lastSpace + 1);
		}

		// Checks if current line start with the EV field.
		if(devices.substr(newLinePos + 1, 5) == "B: EV")
		{
			// The start of the value, while skipping the 'B: EV'.
			std::size_t start{newLinePos + 7};

			newLinePos = devices.find('\n', newLinePos + 1);

			// Grabs the string value of the EV field.
			std::string stringValue{devices.substr(start, newLinePos - start)};

			// Moves the hex string into an uint.
			unsigned int value;
			std::stringstream ss;
			ss << std::hex << stringValue;
			ss >> value;

			// Defines the mask for a keyboard.
			constexpr unsigned int keyboardMask{0x120013};

			if((value & keyboardMask) == keyboardMask)
			{
				break;
			}

			continue;
		}


		newLinePos = devices.find('\n', newLinePos + 1);
	} while(newLinePos != std::string::npos);

	return ev;
}

void* RaspberryInput::ProcessMouseThread(void* arg)
{
	Input* input = reinterpret_cast<Input*>(arg);

	FILE* fmouse;
	fmouse = fopen("/dev/input/mice", "r");

	while(input->isPolling)
	{
		signed char b[3];
		fread(b, sizeof(char), 3, fmouse);

		// if we do plan to scale, best make these into floats for greater precision before they are cast down to ints.
		float mouseX = (float)b[1];
		float mouseY = -(float)b[2];

		input->previousMousePosition = input->mousePosition;
		input->previousPollFrame = input->currentFrame;

		input->mousePosition.x += (mouseX / 1.0f); // 1.0 can be replaced by a scale factor (entierly optional)
		input->mousePosition.y += (mouseY / 1.0f);

		input->mouseDown[MouseButton::Left] = (b[0] & 1) > 0; // using a test( x >0 )  allows it to return and store a bool
		input->mouseDown[MouseButton::Middle] = (b[0] & 4) > 0;
		input->mouseDown[MouseButton::Right] = (b[0] & 2) > 0;
	}
	pthread_exit(nullptr);
}

void* RaspberryInput::ProcessKeyboardThread(void* arg)
{
	Input* input = static_cast<Input*>(arg);

	FILE* fKeyboard;
	fKeyboard = fopen(((Input*)arg)->keyboardLocation.c_str(), "r");

	if(fKeyboard == nullptr)
	{
		printf("Keyboard file not found.");
		pthread_exit(nullptr);
		return nullptr;
	}

	input_event event;

	while(input->isPolling)
	{
		fread(&event, sizeof(input_event), 1, fKeyboard);

		if(event.type == (__u16)EV_KEY)
		{
			input->keyDown[event.code] = event.value > 0;
		}
	}

	fclose(fKeyboard);
	pthread_exit(nullptr);
}
