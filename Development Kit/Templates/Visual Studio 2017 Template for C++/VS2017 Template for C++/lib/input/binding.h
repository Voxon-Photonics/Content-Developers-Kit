#pragma once

#include <functional>

// Link Key Callback to Action Callback
class Binding {
public:
	Binding(std::function<bool()>, std::function<void()>);
	void Test();
	bool IsActive();
	void SetActive(bool);
private:
	bool active = true;
	std::function<bool()> key_bind;
	std::function<void()> callback;
};