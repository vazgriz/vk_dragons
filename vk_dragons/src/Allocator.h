#pragma once

#include <vulkan/vulkan.h>
#include <stack>
#include <vector>
#include <map>
#include <list>
#include <iterator>

struct Allocation {
	VkDeviceMemory memory;
	size_t offset;
	size_t size;
};

struct Node {
	size_t offset;
	size_t size;
};

struct Page {
	VkDeviceMemory memory;
	std::list<Node> nodes;
	void* mapping;
};

class Allocator {
public:
	Allocator(VkDevice device, uint32_t type, size_t pageSize, std::map<VkDeviceMemory, Allocator*>& allocatorMap);

	void Cleanup();

	Allocation Alloc(VkMemoryRequirements requirements);
	void Free(Allocation alloc);
	void Reset();
	uint32_t GetType();
	void* GetMapping(VkDeviceMemory memory);

private:
	VkDevice device;
	size_t pageSize;
	uint32_t type;

	std::vector<Page> pages;
	std::map<VkDeviceMemory, size_t> pageMap;
	std::map<VkDeviceMemory, Allocator*>& allocatorMap;

	void AllocPage();
	Allocation AttemptAlloc(Page& page, VkMemoryRequirements requirements);
	Allocation AttemptAlloc(Page& page, std::list<Node>::iterator iter, VkMemoryRequirements requirements);
	void SplitNode(std::list<Node>& list, std::list<Node>::iterator iter, Allocation alloc);
	void CombineNodes(std::list<Node>& list);
	Page& GetPage(VkDeviceMemory memory);
};