#include "MemorySystem.h"

Memory::Memory(VkPhysicalDevice physicalDevice, VkDevice device) {
	this->device = device;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
}

Memory::~Memory() {
}
