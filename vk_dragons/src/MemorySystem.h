#pragma once
#include <vulkan/vulkan.h>
#include "Allocator.h"
#include <memory>
#include <vector>
#include <map>

//file is named "MemorySystem.h" because "Memory.h" conflicts with included headers in visual studio

#define ALLOCATION_SIZE 128 * 1024 * 1024

class Memory {
public:
	Memory(VkPhysicalDevice physicalDevice, VkDevice device);

	std::unique_ptr<Allocator> hostAllocator;
	Allocator& GetDeviceAllocator(VkMemoryRequirements requirements);
	Allocator& GetDeviceAllocator(uint32_t);

	void Free(Allocation alloc);

	void* GetMapping(VkDeviceMemory memory);

private:
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	std::vector<std::unique_ptr<Allocator>> deviceAllocators;
	std::map<VkDeviceMemory, Allocator*> allocatorMap;

	void AllocHostMemory();
	Allocator& AllocDevice(uint32_t type);
};

