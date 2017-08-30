#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "MeshUtilities.h"
#include "Renderer.h"
#include "MemorySystem.h"
#include "Allocator.h"
#include "ProgramUtilities.h"
#include "Transform.h"
#include "Camera.h"
#include "StagingBuffer.h"

class Model {
public:
	Model(Renderer& renderer, const std::string& fileName);
	~Model();
	void UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Camera& camera);
	void DrawDepth(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	static std::vector<VkVertexInputBindingDescription> GetDepthBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetDepthAttributeDescriptions();

	Transform& GetTransform();

private:
	Renderer& renderer;
	mesh_t mesh;
	uint32_t indexCount;
	std::vector<Buffer> buffers;

	Transform transform;

	void Init(const std::string& fileName);
	void CreateBuffers();
};