#include "Allocator.h"

Allocator::Allocator(VkDeviceMemory memory, uint32_t type, size_t totalSize) {
	this->memory = memory;
	this->totalSize = totalSize;
	this->type = type;
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

	if (pointer > totalSize) throw std::runtime_error("Out of memory");

	return result;
}

void Allocator::Pop() {
	pointer = stack.top();
	stack.pop();
}

void Allocator::Reset() {
	pointer = 0;
	while (!stack.empty()) stack.pop();
}

uint32_t Allocator::GetType() {
	return type;
}