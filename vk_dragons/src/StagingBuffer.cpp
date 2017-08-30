#include "StagingBuffer.h"

StagingBuffer::StagingBuffer(Renderer& renderer, Buffer buffer) : renderer(renderer) {
	this->buffer = buffer;
}

StagingBuffer::~StagingBuffer() {
	renderer.memory->hostAllocator->Free(buffer.alloc);
	vkDestroyBuffer(renderer.device, buffer.buffer, nullptr);
}