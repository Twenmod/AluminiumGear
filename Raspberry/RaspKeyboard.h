#pragma once
#include <IInput.h>
#include <string>

class RaspKeyboard;

struct ThreadData
{
	RaspKeyboard* input;
	std::string keyboardLocation;
};

class RaspKeyboard : public real::IKeyboard
{
public:
	RaspKeyboard();
	bool GetKey(real::Key key) const override;

private:

	std::vector<std::string> keyboardLocations;  // Multiple keyboard locations
	std::vector<pthread_t> keyboardThreads;
	bool* keyDown;

	void FindKeyboardLocations();
	std::vector<std::string> FindActiveKeyboardEvs();

	static void* ProcessKeyboardThread(void* arg);
};

