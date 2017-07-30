#pragma once

#include <vulkan/vulkan.h>

struct Allocation {
	VkDeviceMemory memory;
	size_t offset;
	size_t size;
};

class Allocator {
	//simple linear allocator
public:
	Allocator(VkDeviceMemory memory, size_t totalSize);
	Allocation Alloc(size_t size, size_t alignment);
	void Reset();

private:
	VkDeviceMemory memory;
	size_t totalSize;
	size_t pointer;
};