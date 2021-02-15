#include "voxie_input.h"

#include <functional>
#include "../voxiebox.h"

VoxieInput::VoxieInput()
{
	// Create Out Bindings
	Bindings->AddBinding("Quit", Binding(
		std::bind(&voxieBox::GetKeyDown, &voxieBox::Instance(), K_F4, ALT),
		std::bind(&voxieBox::Quit, &voxieBox::Instance()))
	);

	Bindings->AddBinding("Display2D", Binding(
		std::bind(&voxieBox::GetKeyDown, &voxieBox::Instance(), K_F1, ALT),
		std::bind(&voxieBox::Display2D, &voxieBox::Instance()))
	);

	Bindings->AddBinding("Display3D", Binding(
		std::bind(&voxieBox::GetKeyDown, &voxieBox::Instance(), K_F2, ALT),
		std::bind(&voxieBox::Display3D, &voxieBox::Instance()))
	);
}

std::shared_ptr<Bindable> VoxieInput::GetBindings()
{
	return Bindings;
}
