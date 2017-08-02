#pragma once

#include <vulkan/vulkan.h>
#include <stack>

struct Allocation {
	VkDeviceMemory memory;
	size_t offset;
	size_t size;
};

class Allocator {
	//simple stack allocator
public:
	Allocator(VkDeviceMemory memory, size_t totalSize);
	Allocation Alloc(size_t size, size_t alignment);
	void Free(Allocation allocation);
	void Reset();

private:
	VkDeviceMemory memory;
	size_t totalSize;
	size_t pointer;

	//keeps track of where each allocation starts before alignment is applied, to prevent memory leaks
	std::stack<size_t> stack;
};