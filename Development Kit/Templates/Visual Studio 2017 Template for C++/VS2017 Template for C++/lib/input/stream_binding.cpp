#include "stream_binding.h"

StreamBinding::StreamBinding(std::function<bool()> key, std::function<void(std::string)> action)
{
	key_bind = key;
	callback = action;
}

void StreamBinding::Test(std::function<void(std::function<void(std::string)>)> register_action)
{
	if (key_bind() && active) register_action(callback);
}

bool StreamBinding::IsActive()
{
	return active;
}

void StreamBinding::SetActive(bool new_active)
{
	active = new_active;
}
