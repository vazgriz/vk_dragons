#pragma once
#include <vector>
#include <memory>
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"
#include "Input.h"
#include "Light.h"
#include "Material.h"
#include "UniformBuffer.h"
#include "StagingBuffer.h"

struct CameraUniform {
	glm::mat4 viewProjection;
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 rotationOnlyView;
	glm::mat4 viewInverse;
};

struct LightUniform {
	glm::mat4 viewProjection;
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 position;
	glm::vec4 Ia;
	glm::vec4 Id;
	glm::vec4 Is;
	float shininess;
};

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);
	~Scene();

	void Update(double elapsed);
	void Render();
	void Resize(uint32_t width, uint32_t height);

	uint32_t GetWidth();
	uint32_t GetHeight();

private:
	uint32_t width;
	uint32_t height;
	Renderer renderer;
	Camera camera;
	Input input;
	float time;

	Light light;

	std::unique_ptr<UniformBuffer> camUniform;
	std::unique_ptr<UniformBuffer> lightUniform;

	std::unique_ptr<Model> dragon;
	std::unique_ptr<Model> suzanne;
	std::unique_ptr<Model> plane;
	std::unique_ptr<Model> skybox;
	std::unique_ptr<Model> quad;

	std::unique_ptr<Material> dragonMat;
	std::unique_ptr<Material> suzanneMat;
	std::unique_ptr<Material> planeMat;
	std::unique_ptr<Material> skyboxMat;

	std::unique_ptr<Texture> lightDepth;
	std::shared_ptr<Texture> lightColor;
	std::unique_ptr<Material> lightMat;
	std::shared_ptr<Texture> boxBlur;

	std::unique_ptr<Texture> depth;

	std::shared_ptr<Texture> geometryTarget;
	std::unique_ptr<Material> geometryMat;

	std::shared_ptr<Texture> fxaaTarget;
	std::unique_ptr<Material> fxaaMat;

	VkRenderPass mainRenderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSampler sampler;
	VkDescriptorSetLayout uniformSetLayout;
	VkDescriptorSetLayout modelTextureSetLayout;
	VkDescriptorSetLayout textureSetLayout;

	VkRenderPass lightRenderPass;
	VkFramebuffer lightFramebuffer;

	VkRenderPass boxBlurRenderPass;
	VkFramebuffer boxBlurFramebuffer;

	VkRenderPass geometryRenderPass;
	VkFramebuffer geometryFramebuffer;

	VkRenderPass screenQuadRenderPass;
	VkFramebuffer fxaaFramebuffer;

	void UploadResources(std::vector<std::shared_ptr<Texture>>& textures);
	void UpdateUniform();

	void CreateLightRenderPass();
	void CreateLightFramebuffer();
	void CreateBoxBlurRenderPass();
	void CreateBoxBlurFramebuffer();
	void CreateGeometryRenderPass();
	void CreateGeometryFramebuffer(uint32_t width, uint32_t height);
	void CreateScreenQuadRenderPass();
	void CreateFXAAFramebuffer(uint32_t width, uint32_t height);
	void CreateMainRenderPass();
	void CreateMainFramebuffers(uint32_t width, uint32_t height);
	void AllocateCommandBuffers();
	void RecordCommandBuffer(uint32_t imageIndex);
	void RecordDepthPass(VkCommandBuffer commandBuffer);
	void RecordBoxBlurPass(VkCommandBuffer commandBuffer);
	void RecordGeometryPass(VkCommandBuffer commandBuffer);
	void RecordFXAAPass(VkCommandBuffer commandBuffer);
	void RecordMainPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void CreateSampler();
	void CreateUniformSetLayout();
	void CreateModelTextureSetLayout();
	void CreateTextureSetLayout();

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
	VkPipeline fxaaPipeline;
	VkPipeline finalPipeline;
	void CreatePipelines();
	void DestroyPipelines();
	void RecreatePipelines();
	void CreateModelPipelineLayout();
	void CreateModelPipeline();
	void CreatePlanePipeline();
	void CreateSkyboxPipelineLayout();
	void CreateSkyboxPipeline();
	void CreateLightPipelineLayout();
	void CreateLightPipeline();
	void CreateScreenQuadPipelineLayout();
	void CreateBoxBlurPipeline();
	void CreateFXAAPipeline();
	void CreateFinalPipeline();
};

