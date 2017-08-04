#pragma once
#include <vulkan/vulkan.h>
#include "Allocator.h"
#include <memory>
#include <vector>

//file is named "MemorySystem.h" because "Memory.h" conflicts with included headers in visual studio

#define ALLOCATION_SIZE 64 * 1024 * 1024

class Memory {
public:
	Memory(VkPhysicalDevice physicalDevice, VkDevice device);
	void Cleanup();

	//stack allocators are used because this application has simple memory requirements
	//every resource gets allocated at start up and only full screen buffers need to be allocated after that
	//larger applications will have to use more sophisticated allocation schemes
	std::unique_ptr<Allocator> hostAllocator;
	Allocator& GetDeviceAllocator(VkMemoryRequirements requirements);
	Allocator& GetDeviceAllocator(uint32_t);

	//the mapping for the host memory is kept alive for the entire application
	//it is implicitly unmapped when the memory is freed in Cleanup()
	void* hostMapping;

private:
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VkDeviceMemory hostMemory;
	std::vector<VkDeviceMemory> deviceMemories;
	std::vector<Allocator> deviceAllocators;

	void AllocHostMemory();
	VkDeviceMemory Alloc(uint32_t type);
	Allocator& AllocDevice(uint32_t type);
};

