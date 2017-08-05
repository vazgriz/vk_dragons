#pragma once
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"
#include "Input.h"
#include "DepthBuffer.h"

struct Uniform {
	struct Camera {
		glm::mat4 projection;
		glm::mat4 view;
	} camera;
};

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);
	~Scene();

	void Update(double elapsed);
	void Render();
	void Resize(uint32_t width, uint32_t height);

private:
	Renderer renderer;
	Camera camera;
	Input input;
	float time;

	Model dragon;
	Model suzanne;
	Model plane;
	Texture dragonColor;
	Texture suzanneColor;
	Texture skyboxColor;

	DepthBuffer lightDepth;
	DepthBuffer depth;

	VkRenderPass mainRenderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSampler sampler;
	VkDescriptorSetLayout uniformSetLayout;
	VkDescriptorSetLayout textureSetLayout;
	Buffer uniformBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet uniformSet;
	VkDescriptorSet dragonTextureSet;
	VkDescriptorSet suzanneTextureSet;
	VkDescriptorSet skyboxDescriptorSet;

	void UploadResources();
	void UpdateUniform();

	void createRenderPass();
	void createFramebuffers();
	void AllocateCommandBuffers();
	void RecordCommandBuffer(uint32_t imageIndex);
	void CreateSampler();
	void CreateUniformSetLayout();
	void CreateTextureSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateUniformSet();
	void CreateTextureSet(VkImageView imageView, VkDescriptorSet& descriptorSet);

	void createSwapchainResources(uint32_t width, uint32_t height);
	void CleanupSwapchainResources();

	//defined in Scene_pipelines.cpp
	VkPipelineLayout modelPipelineLayout;
	VkPipeline modelPipeline;
	void CreatePipelines();
	void DestroyPipelines();
	void CreateModelPipelineLayout();
	void CreateModelPipeline();
};

