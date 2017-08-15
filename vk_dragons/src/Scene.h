#pragma once
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"
#include "Input.h"
#include "DepthBuffer.h"
#include "Skybox.h"
#include "Light.h"
#include "ScreenQuad.h"

struct Uniform {
	glm::mat4 camProjection;
	glm::mat4 camView;
	glm::mat4 camRotationOnlyView;
	glm::mat4 camViewInverse;
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::vec4 lightPosition;
	glm::vec4 lightIa;
	glm::vec4 lightId;
	glm::vec4 lightIs;
	float lightShininess;
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
	ScreenQuad quad;

	Texture dragonColor;
	Texture dragonNormal;
	Texture dragonEffects;

	Texture suzanneColor;
	Texture suzanneNormal;
	Texture suzanneEffects;

	Texture planeColor;
	Texture planeNormal;
	Texture planeEffects;

	Texture skyboxColor;
	Texture skyboxSmallColor;

	DepthBuffer lightDepth;
	Texture boxBlur;

	DepthBuffer depth;
	Texture geometryTarget;

	VkRenderPass mainRenderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSampler sampler;
	VkDescriptorSetLayout uniformSetLayout;
	VkDescriptorSetLayout modelTextureSetLayout;
	VkDescriptorSetLayout textureSetLayout;
	Buffer uniformBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet uniformSet;
	VkDescriptorSet dragonTextureSet;
	VkDescriptorSet suzanneTextureSet;
	VkDescriptorSet planeTextureSet;
	VkDescriptorSet skyboxTextureSet;
	VkDescriptorSet lightDepthSet;

	VkRenderPass lightRenderPass;
	VkFramebuffer lightFramebuffer;

	VkRenderPass boxBlurRenderPass;
	VkFramebuffer boxBlurFramebuffer;

	VkRenderPass geometryRenderPass;
	VkFramebuffer geometryFramebuffer;

	VkRenderPass screenQuadRenderPass;

	void UploadResources();
	void UpdateUniform();

	void CreateLightRenderPass();
	void CreateLightFramebuffer();
	void CreateBoxBlurRenderPass();
	void CreateBoxBlurFramebuffer();
	void CreateGeometryRenderPass();
	void CreateGeometryFramebuffer(uint32_t width, uint32_t height);
	void CreateScreenQuadRenderPass();
	void CreateMainRenderPass();
	void CreateMainFramebuffers(uint32_t width, uint32_t height);
	void AllocateCommandBuffers();
	void RecordCommandBuffer(uint32_t imageIndex);
	void RecordDepthPass(VkCommandBuffer commandBuffer);
	void RecordBoxBlurPass(VkCommandBuffer commandBuffer);
	void RecordMainPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void CreateSampler();
	void CreateUniformSetLayout();
	void CreateModelTextureSetLayout();
	void CreateSkyboxSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateUniformSet();
	void CreateTextureSet(VkImageView colorView, VkImageView normalView, VkImageView effectsView, VkDescriptorSet& descriptorSet);
	void CreateSkyboxSet();
	void CreateLightDepthSet();

	void createSwapchainResources(uint32_t width, uint32_t height);
	void CleanupSwapchainResources();

	//defined in Scene_pipelines.cpp
	VkPipelineLayout modelPipelineLayout;
	VkPipelineLayout skyboxPipelineLayout;
	VkPipelineLayout lightPipelineLayout;
	VkPipelineLayout screenQuadPipelineLayout;
	VkPipeline modelPipeline;
	VkPipeline planePipeline;
	VkPipeline skyboxPipeline;
	VkPipeline lightPipeline;
	VkPipeline boxBlurPipeline;
	void CreatePipelines();
	void DestroyPipelines();
	void CreateModelPipelineLayout();
	void CreateModelPipeline();
	void CreatePlanePipeline();
	void CreateSkyboxPipelineLayout();
	void CreateSkyboxPipeline();
	void CreateLightPipelineLayout();
	void CreateLightPipeline();
	void CreateScreenQuadPipelineLayout();
	void CreateBoxBlurPipeline();
};

