#include "Skybox.h"
#include <vector>
#include <glm/glm.hpp>

const std::vector<glm::vec3> positions = {
	{ -1.0, -1.0,  1.0 },
	{  1.0, -1.0,  1.0 },
	{ -1.0,  1.0,  1.0 },
	{  1.0,  1.0,  1.0 },
	{ -1.0, -1.0, -1.0 },
	{  1.0, -1.0, -1.0 },
	{ -1.0,  1.0, -1.0 },
	{  1.0,  1.0, -1.0 },
};

const std::vector<uint32_t> indices = {
	2, 1, 0, 3, 1, 2, // Back face
	3, 5, 1, 7, 5, 3, // Right face
	7, 4, 5, 6, 4, 7, // Front face
	6, 0, 4, 2, 0, 6, // Left face
	1, 4, 0, 5, 4, 1, // Bottom face
	6, 3, 2, 7, 3, 6  // Top face
};

Skybox::Skybox(Renderer& renderer) : renderer(renderer) {
	Init();
}

Skybox::~Skybox() {
	vkDestroyBuffer(renderer.device, vertexBuffer.buffer, nullptr);
	vkDestroyBuffer(renderer.device, indexBuffer.buffer, nullptr);
	renderer.memory->Free(vertexBuffer.alloc);
	renderer.memory->Free(indexBuffer.alloc);
}

void Skybox::Init() {
	vertexBuffer = CreateBuffer(renderer, positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	indexBuffer = CreateBuffer(renderer, indices.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

void Skybox::Draw(VkCommandBuffer commandBuffer) {
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

void Skybox::UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers) {
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, vertexBuffer.buffer, positions.data(), positions.size() * sizeof(glm::vec3))));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, CopyBuffer(renderer, commandBuffer, indexBuffer.buffer, indices.data(), indices.size() * sizeof(uint32_t))));
}

std::vector<VkVertexInputBindingDescription> Skybox::GetBindingDescriptions() {
	return std::vector<VkVertexInputBindingDescription>({
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },
	});
}

std::vector<VkVertexInputAttributeDescription> Skybox::GetAttributeDescriptions() {
	return std::vector<VkVertexInputAttributeDescription>({
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
	});
}
