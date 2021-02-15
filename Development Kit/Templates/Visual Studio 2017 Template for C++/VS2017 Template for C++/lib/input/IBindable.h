#pragma once

#include "binding.h"
#include "stream_binding.h"

#include <map>
#include <string>

// Interface to add Bindability to Class
class Bindable {
public:
	
	void Test(std::function< void(std::function<void(std::string)>)> stream_register);
	
	void AddBinding(std::string binding_name, Binding binding);
	void AddBinding(std::string binding_name, StreamBinding binding);

	void RemoveBinding(std::string binding_name);

	void SetBindingActivity(std::string binding_name, bool is_active);

	bool GetBindingActivity(std::string binding_name);

private:
	bool active = true;
	std::map<std::string, Binding> bindings;
	std::map<std::string, StreamBinding> stream_bindings;
};