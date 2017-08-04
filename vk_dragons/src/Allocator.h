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
	Allocator(VkDeviceMemory memory, uint32_t type, size_t totalSize);
	Allocation Alloc(size_t size, size_t alignment);
	void Pop();
	void Reset();
	uint32_t GetType();

private:
	VkDeviceMemory memory;
	size_t totalSize;
	size_t pointer;
	uint32_t type;

	//keeps track of where each allocation starts before alignment is applied, to prevent memory leaks
	std::stack<size_t> stack;
};