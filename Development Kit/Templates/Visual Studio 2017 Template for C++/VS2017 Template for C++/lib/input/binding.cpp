#include "binding.h"

Binding::Binding(std::function<bool()> key, std::function<void()> call)
{
	key_bind = key;
	callback = call;
}

void Binding::Test()
{
	if (active && key_bind()) callback();
}

bool Binding::IsActive()
{
	return active;
}

void Binding::SetActive(bool new_active)
{
	active = new_active;
}
