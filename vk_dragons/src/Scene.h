#pragma once
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"

struct Uniform {
	glm::mat4 camera;
};

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);
	~Scene();

	void Update();
	void Render();
	void Resize(uint32_t width, uint32_t height);

private:
	Renderer renderer;
	Camera camera;
	Model dragon;
	Model suzanne;
	Model plane;
	Texture dragonColor;
	Texture skybox;

	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorSetLayout descriptorSetLayout;
	Buffer uniformBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	void UploadResources();
	void UpdateUniform();
	void CreateCommandBuffers();
	void CreateDescriptorSetLayout();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
};

