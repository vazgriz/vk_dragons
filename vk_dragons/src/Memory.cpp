#include "MemorySystem.h"
#include <stdexcept>

Memory::Memory(VkPhysicalDevice physicalDevice, VkDevice device) {
	this->device = device;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	//host allocator is created once
	//device allocators are created as needed
	AllocHostMemory();
}

void Memory::Cleanup() {
	hostAllocator->Cleanup();
	for (auto& ptr : deviceAllocators) {
		ptr->Cleanup();
	}
}

void Memory::AllocHostMemory() {
	uint32_t type;
	bool found = false;

	//try to find a memory type that is only HOST_VISIBLE and HOST_COHERENT
	VkMemoryPropertyFlags desiredProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if (memoryProperties.memoryTypes[i].propertyFlags == desiredProperties) {
			type = i;
			found = true;
			break;
		}
	}

	//if that fails, try to find any memory with those flags
	if (!found) {
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & desiredProperties) != 0) {
				type = i;
				found = true;
				break;
			}
		}
	}

	if (!found) throw std::runtime_error("Could not find suitable host memory");

	hostAllocator = std::make_unique<Allocator>(device, type, ALLOCATION_SIZE);
}

Allocator& Memory::AllocDevice(uint32_t type) {
	deviceAllocators.emplace_back(std::make_unique<Allocator>(device, type, ALLOCATION_SIZE));
	return *deviceAllocators[deviceAllocators.size() - 1];
}

Allocator& Memory::GetDeviceAllocator(VkMemoryRequirements requirements) {
	//check if an Allocator that matches the requirements has already been created
	for (auto& ptr : deviceAllocators) {
		auto& allocator = *ptr;
		uint32_t type = allocator.GetType();
		uint32_t test = 1 << type;
		uint32_t bits = requirements.memoryTypeBits & test;
		if (bits != 0) {
			return allocator;
		}
	}

	//if that fails, create Allocator that matches the requirements and is device local
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		uint32_t test = 1 << i;
		if ((requirements.memoryTypeBits & test) != 0 && memoryProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			return AllocDevice(i);
		}
	}

	//if that fails, create Allocator that matches requirements
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		uint32_t test = 1 << i;
		if ((requirements.memoryTypeBits & test) != 0) {
			return AllocDevice(i);
		}
	}

	throw std::runtime_error("Could not find suitable device memory");
}

Allocator& Memory::GetDeviceAllocator(uint32_t type) {
	for (auto& ptr : deviceAllocators) {
		auto& allocator = *ptr;
		if (allocator.GetType() == type) {
			return allocator;
		}
	}

	throw std::runtime_error("Could not find requested allocator");
}

void* Memory::GetMapping(VkDeviceMemory memory) {
	return hostAllocator->GetMapping(memory);
}