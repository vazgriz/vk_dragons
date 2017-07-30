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
	uint32_t hostType = MatchStrict(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (hostType == ~0) hostType = Match(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (hostType == ~0) throw std::runtime_error("Could not find suitable host memory");

	uint32_t deviceType = MatchStrict(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (deviceType == ~0) deviceType = Match(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (deviceType == ~0) throw std::runtime_error("Could not find suitable device memory");

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

	hostAllocator = std::make_unique<Allocator>(hostMemory, ALLOCATION_SIZE);
	deviceAllocator = std::make_unique<Allocator>(deviceMemory, ALLOCATION_SIZE);
}

uint32_t Memory::Match(VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		auto& type = memoryProperties.memoryTypes[i];
		if ((type.propertyFlags & flags) != 0) {
			return i;
		}
	}

	return ~0;
}

uint32_t Memory::MatchStrict(VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		auto& type = memoryProperties.memoryTypes[i];
		if ((type.propertyFlags & flags) == flags) {
			return i;
		}
	}

	return ~0;
}
