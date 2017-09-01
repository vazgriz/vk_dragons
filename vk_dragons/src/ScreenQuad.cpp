#include "ScreenQuad.h"
#include <glm/glm.hpp>

const std::vector<glm::vec3> positions = {
	{ -1.0, -1.0, 0.0 },
	{  1.0, -1.0, 0.0 },
	{ -1.0,  1.0, 0.0 },
	{  1.0,  1.0, 0.0 },
};

const std::vector<uint32_t> indices = {
	0, 1, 2,
	1, 3, 2
};

ScreenQuad::ScreenQuad(Renderer& renderer) : renderer(renderer) {
	Init();
}

ScreenQuad::~ScreenQuad() {
	vkDestroyBuffer(renderer.device, vertexBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, indexBuffer.buffer, nullptr);
	renderer.memory->Free(vertexBuffer.alloc);
	renderer.memory->Free(indexBuffer.alloc);
}

void ScreenQuad::Init() {
	vertexBuffer = CreateBuffer(renderer, positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	indexBuffer = CreateBuffer(renderer, indices.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

void ScreenQuad::Draw(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = {
		vertexBuffer.buffer
	};
	VkDeviceSize offsets[] = {
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void ScreenQuad::UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers) {
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, positions.size() * sizeof(glm::vec3), positions.data()));
	stagingBuffers.back()->CopyToBuffer(commandBuffer, vertexBuffer.buffer);

	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, indices.size() * sizeof(uint32_t), indices.data()));
	stagingBuffers.back()->CopyToBuffer(commandBuffer, indexBuffer.buffer);
}

std::vector<VkVertexInputBindingDescription> ScreenQuad::GetBindingDescriptions() {
	return std::vector<VkVertexInputBindingDescription>({
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },
	});
}

std::vector<VkVertexInputAttributeDescription> ScreenQuad::GetAttributeDescriptions() {
	return std::vector<VkVertexInputAttributeDescription>({
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
	});
}