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
	for (Page& page : pages) {
		Allocation alloc = AttemptAlloc(page, requirements);
		if (alloc.memory != VK_NULL_HANDLE) {
			return alloc;
		}
	}

	//allocate new page
	AllocPage();
	Allocation alloc = AttemptAlloc(pages.back(), requirements);
	if (alloc.memory != VK_NULL_HANDLE) {
		return alloc;
	}

	throw std::runtime_error("Could not allocate memory");
}

void Allocator::Free(Allocation alloc) {
	Page& page = GetPage(alloc.memory);

	for (auto iter = page.nodes.begin(); iter != page.nodes.end(); iter++) {
		if (iter->offset > alloc.offset) {
			page.nodes.insert(iter, { alloc.offset, alloc.size });
			break;
		}
	}

	CombineNodes(page.nodes);
}

void Allocator::Reset() {
	for (Page& page : pages) {
		page.nodes.clear();
		page.nodes.push_back({ 0, pageSize });
	}
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
	pages.back().nodes.push_back({ 0, pageSize });
}

Allocation Allocator::AttemptAlloc(Page& page, VkMemoryRequirements requirements) {
	for (auto iter = page.nodes.begin(); iter != page.nodes.end(); iter++) {
		if (iter->size >= requirements.size) {
			Allocation result = AttemptAlloc(page, iter, requirements);
			if (result.memory != VK_NULL_HANDLE) {
				SplitNode(page.nodes, iter, result);
				return result;
			}
		}
	}

	return { VK_NULL_HANDLE };
}

Allocation Allocator::AttemptAlloc(Page& page, std::list<Node>::iterator iter, VkMemoryRequirements requirements) {
	size_t unalign = iter->offset % requirements.alignment;
	size_t align = 0;

	if (unalign != 0) {
		align = requirements.alignment - unalign;
	}

	if (align + requirements.size > iter->size) return { VK_NULL_HANDLE };

	Allocation result = {
		page.memory,
		iter->offset + align,
		requirements.size
	};

	return result;
}

void Allocator::SplitNode(std::list<Node>& list, std::list<Node>::iterator iter, Allocation alloc) {
	size_t frontSlack = alloc.offset - iter->offset;
	size_t endSlack = (iter->offset + iter->size) - (alloc.offset + alloc.size);

	if (frontSlack == 0 && endSlack == 0) {
		list.erase(iter);
	}
	else if (frontSlack == 0 && endSlack > 0) {
		iter->offset = alloc.offset + alloc.size;
		iter->size = endSlack;
	}
	else if (frontSlack > 0 && endSlack == 0) {
		iter->size = frontSlack;
	}
	else {
		list.insert(iter, { iter->offset, frontSlack });
		iter->offset = alloc.offset + alloc.size;
		iter->size = endSlack;
	}
}

void Allocator::CombineNodes(std::list<Node>& list) {
	for (auto iter = list.begin(); iter != list.end(); iter++) {
		auto next = iter;
		next++;

		if (next != list.end()) {
			if (iter->offset + iter->size == next->offset) {
				iter->size += next->size;
				list.erase(next);
			}
		}
	}
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