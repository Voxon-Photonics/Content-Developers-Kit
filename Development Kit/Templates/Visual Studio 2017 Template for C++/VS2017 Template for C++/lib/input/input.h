#pragma once
#include "input_types.h"
#include "binding.h"
#include "IBindable.h"
#include "../voxiebox.h"
#include <vector>
#include <functional>
#include <memory>

// test

namespace vx {

	enum InputState {
		_IGNORE,
		_BINDING,
		_STREAM
	};

	// Handle Input behaviour
	class Input {

	private:
		InputState _input_state = _BINDING;

		std::vector< std::shared_ptr<Bindable> > registered_bindings;

		std::function< void(std::string) > stream_callback = NULL;
		std::string stream_buffer;

		void ProcessStream();
		void BindStream(std::function<void(std::string)>);
	public:
		Input();
		~Input();
		void Test();
		void AddBinding(std::shared_ptr<Bindable> new_bindings);
	};	
}
