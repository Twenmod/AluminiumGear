#include "precomp.h"
#include "RaspKeyboard.h"

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <linux/input.h>
#include "IInput.h"


//Keyboard was only reading the first device with a keyboard event from proc/bus/input
//After finding this out I used CHATGPT to modify this class to check all input events for keyboards and process input for all of them

RaspKeyboard::RaspKeyboard() : keyDown{ new bool[KEY_MAX] { false } }
{  // KEY_MAX ensures coverage for all possible key codes
	FindKeyboardLocations();

	// Create a thread for each detected keyboard
	for (const auto& keyboardLocation : keyboardLocations)
	{
		pthread_t thread;
		ThreadData* data = new ThreadData{ this, keyboardLocation };  // Bundle RaspKeyboard* and location together
		pthread_create(&thread, nullptr, &ProcessKeyboardThread, data);
		keyboardThreads.push_back(thread);
	}
}

bool RaspKeyboard::GetKey(real::Key key) const
{
	switch(key)
	{
		case real::Key::A: return keyDown[KEY_A];
		case real::Key::B: return keyDown[KEY_B];
		case real::Key::C: return keyDown[KEY_C];
		case real::Key::D: return keyDown[KEY_D];
		case real::Key::E: return keyDown[KEY_E];
		case real::Key::F: return keyDown[KEY_F];
		case real::Key::G: return keyDown[KEY_G];
		case real::Key::H: return keyDown[KEY_H];
		case real::Key::I: return keyDown[KEY_I];
		case real::Key::J: return keyDown[KEY_J];
		case real::Key::K: return keyDown[KEY_K];
		case real::Key::L: return keyDown[KEY_L];
		case real::Key::M: return keyDown[KEY_M];
		case real::Key::N: return keyDown[KEY_N];
		case real::Key::O: return keyDown[KEY_O];
		case real::Key::P: return keyDown[KEY_P];
		case real::Key::Q: return keyDown[KEY_Q];
		case real::Key::R: return keyDown[KEY_R];
		case real::Key::S: return keyDown[KEY_S];
		case real::Key::T: return keyDown[KEY_T];
		case real::Key::U: return keyDown[KEY_U];
		case real::Key::V: return keyDown[KEY_V];
		case real::Key::W: return keyDown[KEY_W];
		case real::Key::X: return keyDown[KEY_X];
		case real::Key::Y: return keyDown[KEY_Y];
		case real::Key::Z: return keyDown[KEY_Z];
		case real::Key::NUM_0: return keyDown[KEY_0];
		case real::Key::NUM_1: return keyDown[KEY_1];
		case real::Key::NUM_2: return keyDown[KEY_2];
		case real::Key::NUM_3: return keyDown[KEY_3];
		case real::Key::NUM_4: return keyDown[KEY_4];
		case real::Key::NUM_5: return keyDown[KEY_5];
		case real::Key::NUM_6: return keyDown[KEY_6];
		case real::Key::NUM_7: return keyDown[KEY_7];
		case real::Key::NUM_8: return keyDown[KEY_8];
		case real::Key::NUM_9: return keyDown[KEY_9];
		case real::Key::TAB: return keyDown[KEY_TAB];
		case real::Key::CAPS_LOCK: return keyDown[KEY_CAPSLOCK];
		case real::Key::SHIFT_LEFT: return keyDown[KEY_LEFTSHIFT];
		case real::Key::CTRL_LEFT: return keyDown[KEY_LEFTCTRL];
		case real::Key::ALT_LEFT: return keyDown[KEY_LEFTALT];
		case real::Key::ESCAPE: return keyDown[KEY_ESC] || keyDown[KEY_HOMEPAGE];
		case real::Key::RIGHT_SHIFT: return keyDown[KEY_RIGHTSHIFT];
		case real::Key::ENTER: return keyDown[KEY_ENTER];
		case real::Key::ARROW_UP: return keyDown[KEY_UP];
		case real::Key::ARROW_RIGHT: return keyDown[KEY_RIGHT];
		case real::Key::ARROW_DOWN: return keyDown[KEY_DOWN];
		case real::Key::ARROW_LEFT: return keyDown[KEY_LEFT];
		case real::Key::SPACE: return keyDown[KEY_SPACE];
		default: std::cout << "ERROR::INPUT::Rasp Keycode not supported: " << static_cast<int>(key) << std::endl; return false;
	}
}

void RaspKeyboard::FindKeyboardLocations()
{
	keyboardLocations = FindActiveKeyboardEvs();
	if (keyboardLocations.empty())
	{
		std::cerr << "No keyboard event found.\n";
	}
}

std::vector<std::string> RaspKeyboard::FindActiveKeyboardEvs()
{
	std::vector<std::string> keyboardPaths;
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir("/dev/input/")) != nullptr)
	{
		while ((ent = readdir(dir)) != nullptr)
		{
			if (strncmp(ent->d_name, "event", 5) == 0)
			{  // Check for event files
				std::string eventPath = "/dev/input/";
				eventPath += ent->d_name;

				// Try opening the event file
				int fd = open(eventPath.c_str(), O_RDONLY);
				if (fd < 0) continue;

				// Check if the device is a keyboard by reading EV_KEY events
				unsigned long evbits[(EV_MAX + 7) / 8];
				memset(evbits, 0, sizeof(evbits));
				ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits);  // Get event bits

				if (evbits[EV_KEY / 8] & (1 << (EV_KEY % 8)))
				{  // Keyboard emits EV_KEY events
					keyboardPaths.push_back(eventPath);  // Found a valid keyboard event
				}

				close(fd);
			}
		}
		closedir(dir);
	}
	return keyboardPaths;
}

void* RaspKeyboard::ProcessKeyboardThread(void* arg)
{
	ThreadData* data = static_cast<ThreadData*>(arg);
	RaspKeyboard* input = data->input;
	const std::string& keyboardLocation = data->keyboardLocation;

	int fd = open(keyboardLocation.c_str(), O_RDONLY);
	if (fd < 0)
	{
		printf("Keyboard file %s not found.\n", keyboardLocation.c_str());
		pthread_exit(nullptr);
		return nullptr;
	}

	input_event event;
	while (true)
	{
		ssize_t bytesRead = read(fd, &event, sizeof(input_event));
		if (bytesRead == sizeof(input_event) && event.type == EV_KEY)
		{
			printf("Key code: %d, Value: %d\n", event.code, event.value);  // Debug output

			// Safeguard against invalid key codes
			if (event.code >= 0 && event.code < KEY_MAX)
			{
				input->keyDown[event.code] = event.value > 0;  // Update the global keyDown state
			}
			else
			{
				printf("Warning: Key code %d out of bounds.\n", event.code);
			}
		}
		else if (bytesRead < 0)
		{
			printf("Error reading input event\n");
			break;
		}
	}

	close(fd);
	delete data;  // Clean up the dynamically allocated thread data
	pthread_exit(nullptr);
}
