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

	//linear allocators are used because this application has simple memory requirements
	//everything gets uploaded to the gpu at start up and nothing needs to be uploaded after that
	//larger applications will have to use more sophisticated allocation schemes
	std::unique_ptr<Allocator> hostAllocator;
	std::unique_ptr<Allocator> deviceAllocator;

	//the mapping for the host memory is kept alive for the entire application
	//it is implicitly unmapped when the memory is freed in Cleanup()
	void* hostMapping;

private:
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VkDeviceMemory hostMemory;
	VkDeviceMemory deviceMemory;

	void AllocMemory();
	uint32_t Match(VkMemoryPropertyFlags flags);
	uint32_t MatchStrict(VkMemoryPropertyFlags flags);
};

