#include "Allocator.h"

Allocator::Allocator(VkDeviceMemory memory, size_t totalSize) {
	this->memory = memory;
	this->totalSize = totalSize;
	pointer = 0;
}

Allocation Allocator::Alloc(size_t size, size_t alignment) {
	stack.push(pointer);
	//move pointer to requested alignment
	size_t unalign = pointer % alignment;
	if (unalign != 0) {
		pointer += (alignment - unalign);
	}

	Allocation result = {
		memory,
		pointer,
		size
	};

	pointer += size;

	return result;
}

void Allocator::Free(Allocation allocation) {
	pointer = stack.top();
	stack.pop();
}

void Allocator::Reset() {
	pointer = 0;
	while (!stack.empty()) stack.pop();
}