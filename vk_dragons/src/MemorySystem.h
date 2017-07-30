#pragma once
#include <vulkan/vulkan.h>

//file is named "MemorySystem.h" because "Memory.h" conflicts with included headers in visual studio

#define ALLOCATION_SIZE 32 * 1024 * 1024

class Memory {
public:
	Memory(VkPhysicalDevice physicalDevice, VkDevice device);
	~Memory();

private:
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;
};

