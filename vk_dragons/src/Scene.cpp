#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height)
	: renderer(window, width, height),
	dragon(renderer),
	suzanne(renderer),
	plane(renderer) {

	dragon.Init("resources/dragon.obj");
	suzanne.Init("resources/suzanne.obj");
	plane.Init("resources/plane.obj");

	renderer.CreateCommandBuffers();
}

void Scene::UploadResources() {
	VkCommandBuffer commandBuffer = renderer.GetSingleUseCommandBuffer();

	dragon.UploadData(commandBuffer);
	suzanne.UploadData(commandBuffer);
	plane.UploadData(commandBuffer);

	renderer.SubmitCommandBuffer(commandBuffer);

	dragon.DestroyStaging();
	suzanne.DestroyStaging();
	plane.DestroyStaging();

	renderer.memory->hostAllocator.reset();
}

void Scene::Update() {

}

void Scene::Render() {
	renderer.Render();
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
}