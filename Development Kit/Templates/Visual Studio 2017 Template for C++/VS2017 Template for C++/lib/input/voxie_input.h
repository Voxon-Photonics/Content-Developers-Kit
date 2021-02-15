#pragma once

#include "IBindable.h"
#include <memory>


class VoxieInput {
public:
	VoxieInput();
	std::shared_ptr<Bindable> GetBindings();
	
private:
	std::shared_ptr<Bindable> Bindings = std::make_shared<Bindable>();
};