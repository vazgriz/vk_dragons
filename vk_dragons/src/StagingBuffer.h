#pragma once
#include "ProgramUtilities.h"

class StagingBuffer {
public:
	StagingBuffer(Renderer& renderer, size_t size, const void* data);
	~StagingBuffer();

	void CopyToBuffer(VkCommandBuffer commandBuffer, VkBuffer dest);
	void CopyToImage(VkCommandBuffer commandBuffer, VkImage dest, uint32_t width, uint32_t height, uint32_t arrayLayer);

private:
	Renderer& renderer;
	size_t size;
	Buffer buffer;

	void* mapping;
};