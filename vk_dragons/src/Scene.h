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

	Model dragon;
	Model suzanne;
	Model plane;
	Texture dragonColor;
	Texture skybox;

	DepthBuffer lightDepth;
	DepthBuffer depth;

	VkRenderPass mainRenderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSampler sampler;
	VkDescriptorSetLayout descriptorSetLayout;
	Buffer uniformBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	void UploadResources();
	void UpdateUniform();

	void createRenderPass();
	void createFramebuffers();
	void CreateCommandBuffers();
	void CreateSampler();
	void CreateDescriptorSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSet();

	void createSwapchainResources(uint32_t width, uint32_t height);
	void CleanupSwapchainResources();

	//defined in Scene_pipelines.cpp
	VkPipelineLayout pipelineLayout;
	VkPipeline dragonPipeline;
	void CreatePipelines();
	void DestroyPipelines();
	void CreateDragonPipeline();
};

