#pragma once
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"

class Scene {
public:
	Scene(GLFWwindow* window, uint32_t width, uint32_t height);

	void Update();
	void Render();
	void Resize(uint32_t width, uint32_t height);

private:
	Renderer renderer;
	Model dragon;
	Model suzanne;
	Model plane;
	Texture dragonColor;
	Texture skybox;

	std::vector<VkCommandBuffer> commandBuffers;

	void UploadResources();
	void CreateCommandBuffers();
};

