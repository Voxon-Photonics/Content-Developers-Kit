#include "IBindable.h"

void Bindable::Test(std::function< void(std::function<void(std::string)>)> stream_register)
{
	if (active) {
		for (auto bind : bindings)
		{
			bind.second.Test();
		}

		for (auto sbind : stream_bindings)
		{
			sbind.second.Test(stream_register);
		}
	}
}

void Bindable::AddBinding(std::string name, Binding bind)
{
	bindings.insert_or_assign(name, bind);
}

void Bindable::AddBinding(std::string name, StreamBinding bind)
{
	stream_bindings.insert_or_assign(name, bind);
}

void Bindable::RemoveBinding(std::string binding_name)
{
	std::map<std::string, Binding>::iterator Bind_it = bindings.find(binding_name);
	if (Bind_it != bindings.end())
	{
		bindings.erase(Bind_it);
	}

	std::map<std::string, StreamBinding>::iterator SBind_it = stream_bindings.find(binding_name);
	if (SBind_it != stream_bindings.end())
	{
		stream_bindings.erase(SBind_it);
	}
}

void Bindable::SetBindingActivity(std::string binding_name, bool is_active)
{
	std::map<std::string, Binding>::iterator Bind_it = bindings.find(binding_name);
	if (Bind_it != bindings.end())
	{
		Bind_it->second.SetActive(is_active);
	}

	std::map<std::string, StreamBinding>::iterator SBind_it = stream_bindings.find(binding_name);
	if (SBind_it != stream_bindings.end())
	{
		SBind_it->second.SetActive(is_active);
	}
}

bool Bindable::GetBindingActivity(std::string binding_name)
{
	bool active = false;

	std::map<std::string, Binding>::iterator Bind_it = bindings.find(binding_name);
	if (Bind_it != bindings.end())
	{
		active |= Bind_it->second.IsActive();
	}

	std::map<std::string, StreamBinding>::iterator SBind_it = stream_bindings.find(binding_name);
	if (SBind_it != stream_bindings.end())
	{
		active |= SBind_it->second.IsActive();
	}

	return active;
}
