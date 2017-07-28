#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height) : renderer(window, width, height) {

}

void Scene::Update() {

}

void Scene::Render() {
	renderer.Render();
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
}