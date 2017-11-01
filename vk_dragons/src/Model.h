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
#include "UniformBuffer.h"
#include "Light.h"

struct ModelUniforms {
	glm::mat4 mvp;
	glm::mat4 mv;
	glm::mat4 lightMVP;
};

//manages vertex and index buffers
class Model {
public:
	Model(Renderer& renderer, const std::string& fileName, VkDescriptorSetLayout layout);
	~Model();
	void UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers);
	void UpdateUniforms(Camera& camera, Light& light);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Camera* camera);
	std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	static std::vector<VkVertexInputBindingDescription> GetDepthBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetDepthAttributeDescriptions();

	Transform& GetTransform();

private:
	Renderer& renderer;
	mesh_t mesh;
	uint32_t indexCount;
	std::vector<Buffer> buffers;

	std::vector<VkBuffer> vkBuffers;
	std::vector<VkDeviceSize> offsets;

	Transform transform;
	std::unique_ptr<UniformBuffer> uniforms;

	void Init(const std::string& fileName);
	void CreateBuffers();
};