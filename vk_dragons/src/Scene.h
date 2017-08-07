#pragma once
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"
#include "Input.h"
#include "DepthBuffer.h"
#include "Skybox.h"
#include "Light.h"

struct Uniform {
	glm::mat4 camProjection;
	glm::mat4 camView;
	glm::mat4 camRotationOnlyView;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
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

	Light light;

	Model dragon;
	Model suzanne;
	Model plane;
	Skybox skybox;
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
	VkDescriptorSet skyboxTextureSet;

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
	VkPipelineLayout skyboxPipelineLayout;
	VkPipeline modelPipeline;
	VkPipeline skyboxPipeline;
	void CreatePipelines();
	void DestroyPipelines();
	void CreateModelPipelineLayout();
	void CreateModelPipeline();
	void CreateSkyboxPipelineLayout();
	void CreateSkyboxPipeline();
};

