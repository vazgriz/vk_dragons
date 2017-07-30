#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height)
	: renderer(window, width, height),
	dragon(renderer),
	suzanne(renderer),
	plane(renderer) {

	dragon.Init("resources/dragon.obj");
	suzanne.Init("resources/suzanne.obj");
	plane.Init("resources/plane.obj");
}

void Scene::UploadResources() {
	VkCommandBuffer commandBuffer = renderer.GetCommandBuffer();

	renderer.SubmitCommandBuffer(commandBuffer);
}

void Scene::Update() {

}

void Scene::Render() {
	renderer.Render();
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
}