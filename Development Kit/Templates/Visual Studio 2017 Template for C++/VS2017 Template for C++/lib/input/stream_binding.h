#pragma once

#include <functional>
#include <string>

// Link Key Callback to Action Callback
class StreamBinding {
public:
	StreamBinding(std::function<bool()>, std::function<void(std::string)>);
	void Test(std::function<void(std::function<void(std::string)>)>);
	bool IsActive();
	void SetActive(bool);
private:
	bool active = true;
	std::function<bool()> key_bind;
	std::function<void(std::string)> callback;
};