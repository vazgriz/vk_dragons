#include "Allocator.h"

Allocator::Allocator(VkDevice device, uint32_t type, size_t pageSize) {
	this->device = device;
	this->pageSize = pageSize;
	this->type = type;
}

void Allocator::Cleanup() {
	for (auto& page : pages) {
		vkFreeMemory(device, page.memory, nullptr);
	}
}

Allocation Allocator::Alloc(VkMemoryRequirements requirements) {
	if (requirements.size > pageSize) throw std::runtime_error("Allocation too large");

	//check existing pages
	for (size_t i = 0; i < pages.size(); i++) {
		Allocation alloc = AttemptAlloc(i, requirements);
		if (alloc.memory != VK_NULL_HANDLE) {
			return alloc;
		}
	}

	//allocate new page
	AllocPage();
	Allocation alloc = AttemptAlloc(pages.size() - 1, requirements);
	if (alloc.memory != VK_NULL_HANDLE) {
		return alloc;
	}

	throw std::runtime_error("Could not allocate memory");
}

void Allocator::Pop() {
	auto internalAlloc = stack.top();
	pages[internalAlloc.page].pointer = internalAlloc.pointer;
	stack.pop();
}

void Allocator::Reset() {
	for (auto& page : pages) {
		page.pointer = 0;
	}
	while (!stack.empty()) stack.pop();
}

uint32_t Allocator::GetType() {
	return type;
}

void Allocator::AllocPage() {
	VkMemoryAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = pageSize;
	info.memoryTypeIndex = type;

	VkDeviceMemory memory;
	VkResult result = vkAllocateMemory(device, &info, nullptr, &memory);

	if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
		throw std::runtime_error("Out of memory");
	}

	pages.push_back({ memory });
	pageMap[memory] = pages.size() - 1;
}

Allocation Allocator::AttemptAlloc(size_t index, VkMemoryRequirements requirements) {
	Page& page = pages[index];
	size_t unalign = page.pointer % requirements.alignment;
	size_t align = 0;

	if (unalign != 0) {
		align = requirements.alignment - unalign;
	}

	if (page.pointer + align + requirements.size > pageSize) return { VK_NULL_HANDLE };

	stack.push({ index, page.pointer });

	Allocation result = {
		page.memory,
		page.pointer + align,
		requirements.size
	};

	page.pointer += align + requirements.size;

	return result;
}

void* Allocator::GetMapping(VkDeviceMemory memory) {
	Page& page = GetPage(memory);
	if (page.mapping != nullptr) return page.mapping;

	VkResult result = vkMapMemory(device, memory, 0, pageSize, 0, &page.mapping);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Could not map memory");
	}

	return page.mapping;
}

Page& Allocator::GetPage(VkDeviceMemory memory) {
	if (pageMap.count(memory) > 0) {
		return pages[pageMap[memory]];
	}

	throw std::runtime_error("Could not find page");
}