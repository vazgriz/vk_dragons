#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(Renderer& renderer, size_t size, VkDescriptorSetLayout layout) : renderer(renderer) {
	this->size = size;
	this->layout = layout;

	buffer = CreateHostBuffer(renderer, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	CreatePool();
	CreateSet();
}

UniformBuffer::~UniformBuffer() {
	renderer.memory->GetHostAllocator().Free(buffer.alloc);
	vkDestroyBuffer(renderer.device, buffer.buffer, nullptr);
	vkDestroyDescriptorPool(renderer.device, pool, nullptr);
}

char* UniformBuffer::GetData() {
	char* base = static_cast<char*>(renderer.memory->GetMapping(buffer.alloc.memory));
	return base + buffer.alloc.offset;
}

void UniformBuffer::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t firstSet) {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, firstSet, 1, &set, 0, nullptr);
}

void UniformBuffer::CreatePool() {
	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.poolSizeCount = 1;
	info.pPoolSizes = &size;
	info.maxSets = 1;

	if (vkCreateDescriptorPool(renderer.device, &info, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor pool");
	}
}

void UniformBuffer::CreateSet() {
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(renderer.device, &info, &set) != VK_SUCCESS) {
		throw std::runtime_error("Could not allocate uniform set");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = set;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(renderer.device, 1, &write, 0, nullptr);
}