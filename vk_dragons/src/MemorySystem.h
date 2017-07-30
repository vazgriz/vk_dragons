#pragma once
#include <vulkan/vulkan.h>
#include "Allocator.h"
#include <memory>

//file is named "MemorySystem.h" because "Memory.h" conflicts with included headers in visual studio

#define ALLOCATION_SIZE 32 * 1024 * 1024

class Memory {
public:
	Memory(VkPhysicalDevice physicalDevice, VkDevice device);
	void Cleanup();

	std::unique_ptr<Allocator> hostAllocator;
	std::unique_ptr<Allocator> deviceAllocator;

private:
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VkDeviceMemory hostMemory;
	VkDeviceMemory deviceMemory;

	void AllocMemory();
	uint32_t Match(VkMemoryPropertyFlags flags);
	uint32_t MatchStrict(VkMemoryPropertyFlags flags);
};

