#include "input.h"


void vx::Input::ProcessStream()
{
	char buf = voxieBox::Instance().GetInputBuffer();

	switch (buf) {
	case 0:
		break;
	case '\r':
	case '\n':
		stream_callback(stream_buffer);
		stream_callback = NULL;
		stream_buffer = std::string();
		_input_state = _BINDING;
		break;
	default:
		stream_buffer += buf;
		break;
	}
}

void vx::Input::BindStream(std::function<void(std::string)> callback)
{
	if (stream_callback == NULL) {
		stream_callback = callback;
		_input_state = _STREAM;
		voxieBox::Instance().ClearInputBuffer();
	}
}

vx::Input::Input()
{
}

vx::Input::~Input()
{
	registered_bindings.clear();
}

void vx::Input::Test()
{
	switch (_input_state) {
		case _BINDING:
			for (std::shared_ptr<Bindable> binding : registered_bindings) {
				// Pass stream_register to tests; let them bind themselves
				binding->Test(std::bind(&vx::Input::BindStream, this, std::placeholders::_1));
			}
			break;
		case _STREAM:
			ProcessStream();	
			break;
		default:
			break;
	}
}

void vx::Input::AddBinding(std::shared_ptr<Bindable> new_bindings)
{
	registered_bindings.push_back(new_bindings);
}
