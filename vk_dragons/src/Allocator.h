#pragma once

#include <vulkan/vulkan.h>
#include <stack>
#include <vector>

struct Allocation {
	VkDeviceMemory memory;
	size_t offset;
	size_t size;
};

struct Page {
	VkDeviceMemory memory;
	size_t pointer;
	void* mapping;
};

struct InternalAllocation {
	size_t page;
	size_t pointer;
};

class Allocator {
public:
	Allocator(VkDevice device, uint32_t type, size_t pageSize);

	void Cleanup();

	Allocation Alloc(size_t size, size_t alignment);
	void Pop();
	void Reset();
	uint32_t GetType();
	void* GetMapping(VkDeviceMemory memory);

private:
	VkDevice device;
	size_t pageSize;
	uint32_t type;

	std::vector<Page> pages;
	std::stack<InternalAllocation> stack;
	void AllocPage();
	Allocation AttemptAlloc(size_t index, size_t size, size_t alignment);
};