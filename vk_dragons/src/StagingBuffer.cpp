#include "StagingBuffer.h"

StagingBuffer::StagingBuffer(Renderer& renderer, size_t size, const void* data) : renderer(renderer) {
	this->size = size;
	buffer = CreateHostBuffer(renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	char* base = static_cast<char*>(renderer.memory->GetMapping(buffer.alloc.memory));
	mapping = base + buffer.alloc.offset;

	memcpy(mapping, data, size);
}

StagingBuffer::~StagingBuffer() {
	renderer.memory->GetHostAllocator().Free(buffer.alloc);
	vkDestroyBuffer(renderer.device, buffer.buffer, nullptr);
}

void StagingBuffer::CopyToBuffer(VkCommandBuffer commandBuffer, VkBuffer dest) {
	VkBufferCopy copy = {};
	copy.size = size;
	copy.srcOffset = 0;	//these control the offset within each buffer
	copy.dstOffset = 0;	//so set each one to zero

	vkCmdCopyBuffer(commandBuffer, buffer.buffer, dest, 1, &copy);
}

void StagingBuffer::CopyToImage(VkCommandBuffer commandBuffer, VkImage dest, uint32_t width, uint32_t height, uint32_t arrayLayer) {
	VkBufferImageCopy copy = {};
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.baseArrayLayer = arrayLayer;
	copy.imageSubresource.layerCount = 1;
	copy.imageOffset = { 0, 0, 0 };
	copy.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer.buffer, dest, VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
}