#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height) : renderer(window, width, height) {
	dragon.Init("resources/dragon.obj");
	suzanne.Init("resources/suzanne.obj");
	plane.Init("resources/plane.obj");
}

void Scene::Update() {

}

void Scene::Render() {
	renderer.Render();
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
}