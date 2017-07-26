#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include <vector>

class Renderer {
public:
	Renderer();
	~Renderer();


private:
	VkInstance instance;

	void CreateInstance();
	bool CheckValidationSupport(const std::vector<const char*>& layers);
};

