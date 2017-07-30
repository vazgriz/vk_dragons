#include "MemorySystem.h"
#include <stdexcept>

Memory::Memory(VkPhysicalDevice physicalDevice, VkDevice device) {
	this->device = device;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	AllocMemory();
}

void Memory::Cleanup() {
	vkFreeMemory(device, hostMemory, nullptr);
	vkFreeMemory(device, deviceMemory, nullptr);
}

void Memory::AllocMemory() {
	//try to find a memory type that is only HOST_VISIBLE and HOST_COHERENT
	//if that fails, try to find any memory with those flags
	//if that fails, raise an exception
	uint32_t hostType = MatchStrict(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (hostType == ~0) hostType = Match(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (hostType == ~0) throw std::runtime_error("Could not find suitable host memory");

	//same as above, try to find a memory type that exactly matches first
	uint32_t deviceType = MatchStrict(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (deviceType == ~0) deviceType = Match(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (deviceType == ~0) throw std::runtime_error("Could not find suitable device memory");

	//the Vulkan spec requires that both of the above types of memory are present, so the exception should never be thrown in this application

	VkMemoryAllocateInfo hostAlloc = {};
	hostAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	hostAlloc.memoryTypeIndex = hostType;
	hostAlloc.allocationSize = ALLOCATION_SIZE;

	VkMemoryAllocateInfo deviceAlloc = {};
	deviceAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	deviceAlloc.memoryTypeIndex = deviceType;
	deviceAlloc.allocationSize = ALLOCATION_SIZE;

	vkAllocateMemory(device, &hostAlloc, nullptr, &hostMemory);
	vkAllocateMemory(device, &deviceAlloc, nullptr, &deviceMemory);
	//these are the only vulkan allocations made in this application

	hostAllocator = std::make_unique<Allocator>(hostMemory, ALLOCATION_SIZE);
	deviceAllocator = std::make_unique<Allocator>(deviceMemory, ALLOCATION_SIZE);

	vkMapMemory(device, hostMemory, 0, ALLOCATION_SIZE, 0, &hostMapping);
}

//finds a memory type that has at least the requested flags
uint32_t Memory::Match(VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		auto& type = memoryProperties.memoryTypes[i];
		if ((type.propertyFlags & flags) != 0) {
			return i;
		}
	}

	return ~0;
}

//finds a memory type that matches the requested flags exactly
uint32_t Memory::MatchStrict(VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		auto& type = memoryProperties.memoryTypes[i];
		if ((type.propertyFlags & flags) == flags) {
			return i;
		}
	}

	return ~0;
}
