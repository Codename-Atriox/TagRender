#include "KeyboardClass.h"

KeyboardClass::KeyboardClass()
{
	for (int i = 0; i < 256; i++)
		this->keyStates[i] = false;
}

bool KeyboardClass::IsKeyPressed(const unsigned char keycode)
{
	return this->keyStates[keycode];
}

bool KeyboardClass::KeyBufferIsEmpty()
{
	return this->keyBuffer.empty();
}
bool KeyboardClass::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
}

KeyboardEvent KeyboardClass::ReadKey()
{
	if (this->keyBuffer.empty())
	{
		return KeyboardEvent(); // no keys = return empty key
	}
	else 
	{
		KeyboardEvent e = this->keyBuffer.front(); // aka first
		this->keyBuffer.pop(); // remove first
		return e;
	}
}
unsigned char KeyboardClass::ReadChar()
{
	if (this->charBuffer.empty())
	{
		return 0;
	}
	else
	{
		unsigned char e = this->charBuffer.front(); // first
		this->charBuffer.pop(); // remove first
		return e;
	}
}

void KeyboardClass::OnKeyPressed(const unsigned char key) 
{
	this->keyStates[key] = true;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Press, key));
}
void KeyboardClass::OnKeyReleased(const unsigned char key)
{
	this->keyStates[key] = false;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Release, key));
}

void KeyboardClass::OnChar(const unsigned char key)
{
	this->charBuffer.push(key);
}

void KeyboardClass::EnableAutoRepeatingKeys() 
{
	this->autoRepeatKeys = true;
}
void KeyboardClass::DisableAuotRepeatingKeys()
{
	this->autoRepeatKeys = false;
}

void KeyboardClass::EnableAutoRepeatingChars()
{
	this->autoRepeatChars = true;
}
void KeyboardClass::DisableAutoRepeatingChars()
{
	this->autoRepeatChars = false;
}

bool KeyboardClass::IsKeysAutoRepeat()
{
	return this->autoRepeatKeys;
}
bool KeyboardClass::IsCharsAutoRepeat()
{
	return this->autoRepeatChars;
}